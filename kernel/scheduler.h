#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define MAX_TASKS 2

typedef struct {
    uint32_t esp;
} task_t;

void scheduler_init();
void scheduler_start();
void scheduler_tick();
void yield();

#endif