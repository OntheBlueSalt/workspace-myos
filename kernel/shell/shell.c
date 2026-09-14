#include "shell.h"
#include "../mm/memory.h"
#include "../drivers/screen.h"
#include "../mm/heap.h"
#include "../fs/ramfs.h"
#include "../lib/string.h"


#define LINE_MAX    128

static char line_buffer[LINE_MAX];
static int line_length = 0;
static volatile int line_ready = 0;
static fs_node_t *cwd = 0;


// 键盘中断里调用：把字符放进缓存区
void shell_input_char(char c)
{
    if ( c == '\n')
    {
        line_buffer[line_length] = '\0';
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
    if (str_eq(cmd, "help")) {
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
    } else if (str_eq(cmd, "mem")) {
        cmd_mem();
    } else if (str_eq(cmd, "clear") || str_eq(cmd, "clean")) {
        clear_screen();
    } else if (str_eq(cmd, "echo")) {
        print_char('\n');
    } else if (str_starts_with(cmd, "echo ")) {
        print_line(cmd + 5);
    } else if (cmd[0] == '\0') {
        // 空行
    } else if (str_eq(cmd, "ls")) {
        char path[128];
        ramfs_get_path(cwd, path, sizeof(path));
        ramfs_list(path);
    } else if (str_starts_with(cmd, "ls ")) {
        char abs[128];
        make_abs(cmd + 3, abs, sizeof(abs));
        ramfs_list(abs);
    } else if (str_eq(cmd, "pwd")) {
        ramfs_pwd(cwd);
    } else if (str_eq(cmd, "cd")) {
        cwd = ramfs_root();
    } else if (str_starts_with(cmd, "cd ")) {
        char abs[128];
        make_abs(cmd + 3, abs, sizeof(abs));
        fs_node_t *target = ramfs_lookup(abs);
        if (target && target->type == NODE_DIR)
            cwd = target;
        else
            print_line("no such directory");
    } else if (str_starts_with(cmd, "mkdir ")) {
        char abs[128];
        make_abs(cmd + 6, abs, sizeof(abs));
        if (ramfs_mkdir(abs) == 0)
            print_line("created dir");
        else
            print_line("failed");
    } else if (str_starts_with(cmd, "touch ")) {
        char abs[128];
        make_abs(cmd + 6, abs, sizeof(abs));
        if (ramfs_create(abs) == 0)
            print_line("created");
        else
            print_line("failed");
    } else if (str_starts_with(cmd, "cat ")) {
        char abs[128];
        make_abs(cmd + 4, abs, sizeof(abs));
        char buf[1024];
        if (ramfs_read(abs, buf, sizeof(buf)) >= 0)
            print_line(buf);
        else
            print_line("no such file");
    } else if (str_starts_with(cmd, "rm ")) {
        char abs[128];
        make_abs(cmd + 3, abs, sizeof(abs));
        if (ramfs_delete(abs) == 0)
            print_line("deleted");
        else
            print_line("no such file or not empty");
    } else if (str_starts_with(cmd, "write ")) {
        const char *p = cmd + 6;
        char rel[64];
        int n = 0;
        while (*p && *p != ' ' && n < 63) rel[n++] = *p++;
        rel[n] = '\0';
        if (*p == ' ') p++;
        char abs[128];
        make_abs(rel, abs, sizeof(abs));
        if (ramfs_write(abs, p) == 0)
            print_line("written");
        else
            print_line("no such file");
    } else {
        print_string("Unknown command: ");
        print_line(cmd);
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
