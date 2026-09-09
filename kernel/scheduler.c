#include "scheduler.h"
#include <stdint.h>

void task_one();
void task_two();

task_t tasks[MAX_TASKS];
int current_task = 0;

#define STACK_SIZE 4096
uint32_t stack_one[STACK_SIZE / 4];
uint32_t stack_two[STACK_SIZE / 4];

extern void switch_to(uint32_t *old_esp, uint32_t new_esp);
extern void start_first_task(uint32_t esp);

void setup_task_stack(int task_id, void(*task_func)()) {
    uint32_t *stack;
    if (task_id == 0) {
        stack = stack_one + STACK_SIZE / 4;
    } else {
        stack = stack_two + STACK_SIZE / 4;
    }

    *(--stack) = (uint32_t)task_func;

    for (int i = 0; i < 8; i++) {
        *(--stack) = 0;
    }

    tasks[task_id].esp = (uint32_t)stack;
}

void scheduler_init() {
    setup_task_stack(0, task_one);
    setup_task_stack(1, task_two);
    current_task = 0;
}

void scheduler_start() {
    asm volatile("sti");   // 开中断（键盘仍可使用）
    start_first_task(tasks[0].esp);
}

void yield() {
    int next_task = (current_task + 1) % MAX_TASKS;
    task_t *old_task = &tasks[current_task];
    current_task = next_task;
    switch_to(&old_task->esp, tasks[next_task].esp);
}

void task_one() {
    while (1) {
        char *video = (char *) 0xB8000;
        video[320] = 'A';
        video[321] = 0x0F;
        for (volatile int i = 0; i < 1000000; i++);
        yield();
    }
}

void task_two() {
    while (1) {
        char *video = (char *) 0xB8000;
        video[320] = 'B';
        video[321] = 0x0F;
        for (volatile int i = 0; i < 1000000; i++);
        yield();
    }
}