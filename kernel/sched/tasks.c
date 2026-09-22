#include "scheduler.h"
#include "../lib/stdint.h"

#define POS(row, col) ((row * 80 + col) * 2)

void task_a()
{
    while (1) {
        char *video = (char *)0xB8000;
        video[POS(24, 70)] = 'A';
        video[POS(24, 70) + 1] = 0x0F;
        for (volatile int i = 0; i < 5000000; i++);
    }
}

void task_b()
{
    while (1) {
        char *video = (char *)0xB8000;
        video[POS(24, 72)] = 'B';
        video[POS(24, 72) + 1] = 0x0F;
        for (volatile int i = 0; i < 5000000; i++);
    }
}

void task_c()
{
    while (1) {
        char *video = (char *)0xB8000;
        video[POS(24, 74)] = 'C';
        video[POS(24, 74) + 1] = 0x0F;
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
    char *video = (char *)0xB8000;
    while (1)
    {
        video[POS(24, 78)] = 'S';
        video[POS(24, 78) + 1] = 0x0F;
        task_sleep(1000);
    }
}