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


// 执行一条命令
static void execute(const char *cmd) {
    if (str_eq(cmd, "help")) {
        print_line("Commands:");
        print_line("  help        - show this");
        print_line("  mem         - memory info");
        print_line("  clear       - clear screen");
        print_line("  echo X      - print X");
        print_line("  ls [PATH]   - list directory");
        print_line("  mkdir PATH  - create directory");
        print_line("  touch PATH  - create file");
        print_line("  cat PATH    - show file");
        print_line("  write PATH CONTENT - write file");
        print_line("  rm PATH     - delete file/dir");
    } else if (str_eq(cmd, "mem")) {
        cmd_mem();
    } else if (str_eq(cmd, "clear")) {
        clear_screen();
    } else if (str_eq(cmd, "echo")) {
        print_char('\n');
    } else if (str_starts_with(cmd, "echo ")) {
        print_line(cmd + 5);
    } else if (cmd[0] == '\0') {
        // 空行
    } else if (str_eq(cmd, "ls")) {
        ramfs_list("/");
    } else if (str_starts_with(cmd, "ls ")) {
        ramfs_list(cmd + 3);
    } else if (str_starts_with(cmd, "mkdir ")) {
        void *p = kmalloc(64);
        if (p) print_line("kmalloc ok");
        else   print_line("kmalloc failed");
    } else if (str_starts_with(cmd, "touch ")) {
        if (ramfs_create(cmd + 6) == 0)
            print_line("created");
        else
            print_line("failed");
    } else if (str_starts_with(cmd, "cat ")) {
        char buf[1024];
        if (ramfs_read(cmd + 4, buf, sizeof(buf)) >= 0)
            print_line(buf);
        else
            print_line("no such file");
    } else if (str_starts_with(cmd, "rm ")) {
        if (ramfs_delete(cmd + 3) == 0)
            print_line("deleted");
        else
            print_line("no such file or not empty");
    } else if (str_starts_with(cmd, "write ")) {
        const char *p = cmd + 6;
        char path[64];
        int n = 0;
        while (*p && *p != ' ' && n < 63) path[n++] = *p++;
        path[n] = '\0';
        if (*p == ' ') p++;
        if (ramfs_write(path, p) == 0)
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
