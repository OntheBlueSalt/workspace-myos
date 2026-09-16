#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../lib/stdint.h"

#define MAX_TASKS   8
#define TASK_NAME_MAX   16

typedef enum
{
    TASK_UNUSED = 0,
    TASK_RUNNING = 1,
    TASK_READY = 2,
    TASK_END = 3
} task_state_t;

typedef struct {
    uint32_t        esp;
    uint32_t        stack_base;
    int             pid;
    char            name[TASK_NAME_MAX];
    task_state_t    state;
} task_t;

void        scheduler_init();
int         scheduler_create_task(const char *task_name, void (*entry)());
void        scheduler_start();
uint32_t    scheduler_tick(uint32_t old_esp);
void        task_exit();
void        scheduler_ps();
int         scheduler_kill(int pid);

#endif