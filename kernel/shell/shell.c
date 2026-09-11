#include "shell.h"
#include "../mm/memory.h"
#include "../drivers/screen.h"
#include "../mm/heap.h"
#include "../fs/ramfs.h"


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

// 比较字符串是否相等
static int str_eq( const char *a, const char *b )
{
    while (*a && *b)
    {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
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


//
static int str_starts_with( const char *str, const char *prefix)
{
    while (*prefix)
    {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}


// 执行一条命令
static void execute(const char *cmd)
{
    if (str_eq(cmd, "help"))
    {
        print_line("Commands:");
        print_line("  help  - show this");
        print_line("  mem   - memory info");
        print_line("  clear - clear screen");
        print_line("  echo X- print X");
        print_line("  ls         - list files");
        print_line("  touch NAME - create file");
        print_line("  cat NAME   - show file");
        print_line("  write NAME CONTENT - write file");
        print_line("  rm NAME    - delete file");
    } else if (str_eq(cmd, "mem"))
    {
        cmd_mem();
    }else if (str_eq(cmd, "clear"))
    {
        clear_screen();
    } else if (str_eq(cmd, "echo"))
    {
        print_line("");
    }else if (str_starts_with(cmd, "echo "))
    {
        print_line(cmd + 5);
    } else if (cmd[0] == '\0')
    {

    } else if (str_eq(cmd, "ls"))
    {
        ramfs_list();
    } else if (str_starts_with(cmd, "touch "))
    {
        if (ramfs_create(cmd + 6) == 0)
            print_line("created");
        else
            print_line("failed");
    } else if (str_starts_with(cmd, "cat "))
    {
        char buf[1024];
        if (ramfs_read(cmd + 4, buf, sizeof(buf)) >= 0)
            print_line(buf);
        else
            print_line("no such file");
    } else if (str_starts_with(cmd, "rm "))
    {
        if (ramfs_delete(cmd + 3) == 0)
            print_line("deleted");
        else
            print_line("no such file");
    } else if (str_starts_with(cmd, "write "))
    {
        const char *p = cmd + 6;
        char name[32];
        int n = 0;
        while (*p && *p != ' ' && n < 31) name[n++] = *p++;
        name[n] = '\0';
        if (*p == ' ') p++;   // 跳过空格
        if (ramfs_write(name, p) == 0)
            print_line("written");
        else
            print_line("no such file");
    } else
    {
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
