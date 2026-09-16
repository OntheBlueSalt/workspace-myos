#include "shell.h"
#include "../mm/memory.h"
#include "../drivers/screen.h"
#include "../mm/heap.h"
#include "../fs/ramfs.h"
#include "../lib/string.h"
#include "../sched/scheduler.h"
#include "../sched/tasks.h"


#define LINE_MAX    128
#define HIST_MAX    8

static char line_buffer[LINE_MAX];
static int line_length = 0;
static volatile int line_ready = 0;
static fs_node_t *cwd = 0;
static char history[HIST_MAX][LINE_MAX];
static int hist_count = 0;
static int hist_index = -1;


// 键盘中断里调用：把字符放进缓存区
void shell_input_char(char c)
{
    if ( c == 0x03) // Ctrl + C
    {
        print_line("^C");
        line_length = 0;
        line_ready = 1;
        return;
    }
    if ( c == 0x0C) // Ctrl + L
    {
        clear_screen();
        print_string("> ");
        for (int i = 0; i < line_length; i++)
        {
            print_char(line_buffer[i]);
        }
        return;
    }
    if ( c == '\n')
    {
        line_buffer[line_length] = '\0';
        if (line_length > 0) {
            if (hist_count < HIST_MAX) {
                str_copy(history[hist_count], line_buffer, LINE_MAX);
                hist_count++;
            } else {
                for (int i = 0; i < HIST_MAX - 1; i++)
                    str_copy(history[i], history[i + 1], LINE_MAX);
                str_copy(history[HIST_MAX - 1], line_buffer, LINE_MAX);
            }
        }

        hist_index = -1;
        line_ready = 1;
        print_char('\n');
        return;
    }
    if ( c == '\b')
    {
        if (line_length > 0)
        {
            line_length--;
            backspace();
        }
        return;
    }
    if (line_length < LINE_MAX - 1)
    {
        line_buffer[line_length++] = c;
        print_char(c);
    }
}

void shell_history_up()
{
    if (hist_count == 0) return;
    if (hist_index == -1) hist_index = hist_count - 1;
    else if (hist_index > 0) hist_index--;

    while (line_length > 0)
    {
        line_length--;
        backspace();
    }
    int n = 0;
    while (history[hist_index][n])
    {
        line_buffer[line_length++] = history[hist_index][n];
        print_char(history[hist_index][n]);
        n++;
    }
}

void shell_history_down()
{
    if (hist_index == -1) return;

    if (hist_index < hist_count - 1)
    {
        hist_index++;
        while (line_length > 0)
        {
            line_length--;
            backspace();
        }
        int n = 0;
        while (history[hist_index][n])
        {
            line_buffer[line_length++] = history[hist_index][n];
            print_char(history[hist_index][n]);
            n++;
        }
    } else
    {
        hist_index = -1;
        while (line_length > 0)
        {
            line_length--;
            backspace();
        }
    }
}

void shell_tab_complete()
{
    static const char *cmds[] =
    {
        "help", "mem", "clear", "clean", "echo",
        "ls", "mkdir", "touch", "cat", "write",
        "rm", "cd", "pwd"
    };
    int n = sizeof(cmds) / sizeof(cmds[0]);

    // 只在光标位于行尾且只输入了第一个单词时补全
    if (line_length == 0) return;

    // 检查 line_buffer 中是否已有空格（说明在输入参数，不做命令补全）
    for (int i = 0; i < line_length; i++)
    {
        if (line_buffer[i] == ' ') return;
    }

    int match_count = 0;
    const char *match = 0;
    for (int i = 0; i < n; i++)
    {
        if (str_starts_with(cmds[i], line_buffer)) {
            match = cmds[i];
            match_count++;
        }
    }

    if (match_count == 1)
    {
        // 唯一匹配，补全
        while (line_length > 0)
        {
            line_length--;
            backspace();
        }
        int k = 0;
        while (match[k])
        {
            line_buffer[line_length++] = match[k];
            print_char(match[k]);
            k++;
        }
        line_buffer[line_length++] = ' ';
        print_char(' ');
    } else if (match_count > 1)
    {
        // 多个匹配，全部列出
        print_char('\n');
        for (int i = 0; i < n; i++)
        {
            if (str_starts_with(cmds[i], line_buffer)) {
                print_string(cmds[i]);
                print_char(' ');
            }
        }
        print_line("");
        print_string("> ");
        for (int i = 0; i < line_length; i++)
        {
            print_char(line_buffer[i]);
        }
    }
}

// 打印内存信息
static void cmd_mem()
{
    print_string("Free pages:");
    print_dec(get_free_page_count());
    print_char('\n');
    print_string("Heap free: ");
    print_dec(heap_free_bytes());
    print_char('\n');
}

static void make_abs(const char *in, char *out, int outsize)
{
    if (in[0] == '/') {
        int i = 0;
        while (in[i] && i < outsize - 1) { out[i] = in[i]; i++; }
        out[i] = '\0';
        return;
    }
    char cwd_path[128];
    ramfs_get_path(cwd, cwd_path, sizeof(cwd_path));
    int i = 0;
    while (cwd_path[i] && i < outsize - 1) { out[i] = cwd_path[i]; i++; }
    if (i > 0 && out[i - 1] != '/') {
        if (i < outsize - 1) out[i++] = '/';
    }
    int j = 0;
    while (in[j] && i < outsize - 1) { out[i++] = in[j++]; }
    out[i] = '\0';
}


// 执行一条命令
static void execute(const char *cmd) {
    // 拷贝并裁掉末尾空格
    char buf[LINE_MAX];
    int n = 0;
    while (*cmd && n < LINE_MAX - 1) {
        buf[n++] = *cmd++;
    }
    buf[n] = '\0';
    while (n > 0 && buf[n - 1] == ' ') {
        buf[n - 1] = '\0';
        n--;
    }

    if (str_eq(buf, "help")) {
        print_line("Commands:");
        print_line("  help        - show this");
        print_line("  mem         - memory info");
        print_line("  clear|clean - clear screen");
        print_line("  echo X      - print X");
        print_line("  ls [PATH]   - list directory");
        print_line("  mkdir PATH  - create directory");
        print_line("  touch PATH  - create file");
        print_line("  cat PATH    - show file");
        print_line("  write PATH CONTENT - write file");
        print_line("  rm PATH     - delete file/dir");
        print_line("  cd [PATH]   - change directory");
        print_line("  pwd         - print working directory");
        print_line("  run a|b|c   - create task");
        print_line("  start       - start first task");
        print_line("  ps          - list tasks");
        print_line("  kill PID    - kill task");
    } else if (str_eq(buf, "mem")) {
        cmd_mem();
    } else if (str_eq(buf, "clear") || str_eq(buf, "clean")) {
        clear_screen();
    } else if (str_eq(buf, "echo")) {
        print_char('\n');
    } else if (str_starts_with(buf, "echo ")) {
        print_line(buf + 5);
    } else if (buf[0] == '\0') {
        // 空行
    } else if (str_eq(buf, "ls")) {
        char path[128];
        ramfs_get_path(cwd, path, sizeof(path));
        ramfs_list(path);
    } else if (str_starts_with(buf, "ls ")) {
        char abs[128];
        make_abs(buf + 3, abs, sizeof(abs));
        ramfs_list(abs);
    } else if (str_eq(buf, "pwd")) {
        ramfs_pwd(cwd);
    } else if (str_eq(buf, "cd")) {
        cwd = ramfs_root();
    } else if (str_starts_with(buf, "cd ")) {
        char abs[128];
        make_abs(buf + 3, abs, sizeof(abs));
        fs_node_t *target = ramfs_lookup(abs);
        if (target && target->type == NODE_DIR)
            cwd = target;
        else
            print_line("no such directory");
    } else if (str_starts_with(buf, "mkdir ")) {
        char abs[128];
        make_abs(buf + 6, abs, sizeof(abs));
        if (ramfs_mkdir(abs) == 0)
            print_line("created dir");
        else
            print_line("failed");
    } else if (str_starts_with(buf, "touch ")) {
        char abs[128];
        make_abs(buf + 6, abs, sizeof(abs));
        if (ramfs_create(abs) == 0)
            print_line("created");
        else
            print_line("failed");
    } else if (str_starts_with(buf, "cat ")) {
        char abs[128];
        make_abs(buf + 4, abs, sizeof(abs));
        char content[1024];
        if (ramfs_read(abs, content, sizeof(content)) >= 0)
            print_line(content);
        else
            print_line("no such file");
    } else if (str_starts_with(buf, "rm ")) {
        char abs[128];
        make_abs(buf + 3, abs, sizeof(abs));
        if (ramfs_delete(abs) == 0)
            print_line("deleted");
        else
            print_line("no such file or not empty");
    } else if (str_starts_with(buf, "write ")) {
        const char *p = buf + 6;
        char rel[64];
        int m = 0;
        while (*p && *p != ' ' && m < 63) rel[m++] = *p++;
        rel[m] = '\0';
        if (*p == ' ') p++;
        char abs[128];
        make_abs(rel, abs, sizeof(abs));
        if (ramfs_write(abs, p) == 0)
            print_line("written");
        else
            print_line("no such file");
    } else if (str_eq(buf, "run a")) {
        int pid = scheduler_create_task("A", task_a);
        if (pid > 0) { print_string("started task "); print_dec(pid); print_char('\n'); }
        else print_line("failed");
    } else if (str_eq(buf, "run b")) {
        int pid = scheduler_create_task("B", task_b);
        if (pid > 0) { print_string("started task "); print_dec(pid); print_char('\n'); }
        else print_line("failed");
    } else if (str_eq(buf, "run c")) {
        int pid = scheduler_create_task("C", task_c);
        if (pid > 0) { print_string("started task "); print_dec(pid); print_char('\n'); }
        else print_line("failed");
    } else if (str_eq(buf, "run s")) {
        int pid = scheduler_create_task("S", task_short);
        if (pid > 0) { print_string("started task "); print_dec(pid); print_char('\n'); }
        else print_line("failed");
    } else if (str_eq(buf, "run")) {
        print_line("usage: run a|b|c");
    } else if (str_starts_with(buf, "run ")) {
        print_line("unknown task");
    } else if (str_eq(buf, "ps")) {
        scheduler_ps();
    } else if (str_eq(buf, "start")) {
        scheduler_start();
    } else if (str_starts_with(buf, "kill ")) {
        int pid = 0;
        const char *p = buf + 5;
        while (*p >= '0' && *p <= '9') {
            pid = pid * 10 + (*p - '0');
            p++;
        }
        if (scheduler_kill(pid) == 0)
            print_line("killed");
        else
            print_line("no such pid");
    } else {
        print_string("Unknown command: ");
        print_line(buf);
    }
}

void shell_run()
{
    cwd = ramfs_root();
    print_string("> ");
    while (1)
    {
        if (line_ready)
        {
            line_ready = 0;
            execute(line_buffer);
            line_length = 0;
            print_string("> ");
        }
        asm volatile("hlt");
    }
}
