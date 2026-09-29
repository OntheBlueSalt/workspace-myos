#include "scheduler.h"
#include "../lib/stdint.h"
#include "../syscall/syscall.h"

#define POS(row, col) ((row * 80 + col) * 2)

void task_a()
{
    while (1) {
        sys_print_at('A', 70);
        for (volatile int i = 0; i < 5000000; i++);
    }
}

void task_b()
{
    while (1) {
        sys_print_at('B', 72);
        for (volatile int i = 0; i < 5000000; i++);
    }
}

void task_c()
{
    while (1) {
        sys_print_at('C', 74);
        for (volatile int i = 0; i < 5000000; i++);
    }
}

// 只跑 5 次就结束
void task_short()
{
    for (int n = 0; n < 5; n++) {
        char *video = (char *)0xB8000;
        video[POS(24, 76)] = '0' + n;
        video[POS(24, 76) + 1] = 0x0F;
        for (volatile int i = 0; i < 5000000; i++);
    }
    task_exit();
}

// sleep任务
void task_sleeper()
{
    while (1)
    {
        sys_print_at('S', 78);
        sys_sleep(1000);
    }
}