#include "syscall.h"
#include "../drivers/screen.h"
#include "../sched/scheduler.h"

extern void syscall_entry();

// 内核态系统调用分发
uint32_t syscall_dispatch(uint32_t num, uint32_t a, uint32_t b, uint32_t c)
{
    (void)b;
    (void)c;

    switch (num)
    {
        case SYS_PRINT:
            char *video = (char *)0xB8000;
            int col = (int)b;
            if (col < 0) col = 0;
            if (col > 79) col = 79;
            int pos = (24 * 80 + col) * 2;
            video[pos] = (char)a;
            video[pos + 1] = 0x0F;
            return 0;
        case SYS_EXIT:
            task_exit();
            return 0;
        case SYS_SLEEP:
            task_sleep(a);
            return 0;
        case SYS_GETPID:
            return current_task_pid();
        default:
            return (uint32_t)-1;
    }
}

void syscall_init()
{
    // 设置 IDT 0x80 号中断
    extern void idt_set_gate(int num, uint32_t base, uint16_t selector, uint8_t flags);
    idt_set_gate(0x80, (uint32_t)syscall_entry, 0x08, 0xEF);
    // 0xEE = 32 位中断门， DPL=3(允许用户态调用)
}

// 任务侧封装
void sys_print(char c)
{
    asm volatile("int $0x80"::"a"(SYS_PRINT),"b"((uint32_t)c));
}

void sys_exit()
{
    asm volatile("int $0x80"::"a"(SYS_EXIT));
}

void sys_sleep(uint32_t ms)
{
    asm volatile("int $0x80"::"a"(SYS_SLEEP),"b"(ms));
}

int sys_getpid()
{
    uint32_t ret;
    asm volatile("int $0x80":"=a"(ret):"a"(SYS_GETPID));
    return (int)ret;
}

void sys_print_at(char c, int col)
{
    asm volatile("int $0x80" : : "a"(SYS_PRINT), "b"((uint32_t)c), "c"((uint32_t)col));
}