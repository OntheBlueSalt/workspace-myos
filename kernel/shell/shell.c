#include "shell.h"
#include "../mm/memory.h"
#include "../drivers/screen.h"
#include "../mm/heap.h"


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
