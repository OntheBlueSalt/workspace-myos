#include "scheduler.h"
#include <stdint.h>

void task_a() {
    while (1) {
        char *video = (char *)0xB8000;
        video[320] = 'A';
        video[321] = 0x0F;
        video[322] = '1';   // 额外标志
        video[323] = 0x0F;
        for (volatile int i = 0; i < 1000000; i++);
        yield();
    }
}

void task_b() {
    while (1) {
        char *video = (char *)0xB8000;
        video[324] = 'B';
        video[325] = 0x0F;
        video[326] = '2';   // 额外标志
        video[327] = 0x0F;
        for (volatile int i = 0; i < 1000000; i++);
        yield();
    }
}

void task_c() {
    while (1) {
        char *video = (char *)0xB8000;
        video[328] = 'C';
        video[329] = 0x0F;
        for (volatile int i = 0; i < 1000000; i++);
        yield();
    }
}