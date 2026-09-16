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
        video[POS(24, 70)] = 'B';
        video[POS(24, 70) + 1] = 0x0F;
        for (volatile int i = 0; i < 5000000; i++);
    }
}

void task_c()
{
    while (1) {
        char *video = (char *)0xB8000;
        video[POS(24, 70)] = 'C';
        video[POS(24, 70) + 1] = 0x0F;
        for (volatile int i = 0; i < 5000000; i++);
    }
}