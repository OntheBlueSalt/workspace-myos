#include "scheduler.h"
#include "../mm/heap.h"
#include "../drivers/screen.h"
#include "../lib/string.h"

static task_t tasks[MAX_TASKS];
static int task_count = 0;
static int current_task = 0;

static volatile uint32_t global_tick = 0;

#define STACK_SIZE 2048
#define TICK_MS    10

extern void start_first_task(uint32_t esp);

static void setup_task_stack(task_t *t, void (*entry)())
{
    uint32_t *stack = kmalloc(STACK_SIZE);
    if (!stack) return;

    t->stack_base = (uint32_t)stack;

    uint32_t *top = stack + STACK_SIZE / 4;

    // 构造中断帧 iretd 会弹出 EIP | CS | EFLAGS
    *--top = 0x202;
    *--top = 0x08;
    *--top = (uint32_t)entry;

    for (int i = 0; i < 8; i++)
    {
        *--top = 0;
    }

    t->esp = (uint32_t)top;
}

void scheduler_init()
{
    for (int i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].state = TASK_UNUSED;
        tasks[i].pid = 0;
        tasks[i].esp = 0;
        tasks[i].stack_base = 0;
        tasks[i].wake_tick = 0;
        tasks[i].name[0] = '\0';
    }
    task_count = 0;
    current_task = 0;
}

int scheduler_create_task(const char *name, void (*entry)())
{
    // 找一个空闲槽
    int id = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            id = i;
            break;
        }
    }
    if (id == -1) return -1;

    tasks[id].pid = id + 1;
    str_copy(tasks[id].name, name, TASK_NAME_MAX);
    tasks[id].state = TASK_READY;
    tasks[id].esp = 0;
    tasks[id].stack_base = 0;
    setup_task_stack(&tasks[id], entry);

    if (tasks[id].esp == 0) {
        tasks[id].state = TASK_UNUSED;
        return -1;
    }
    if (id >= task_count) task_count = id + 1;
    return tasks[id].pid;
}

void scheduler_start()
{
    if (task_count == 0) return;
    tasks[0].state = TASK_RUNNING;
    current_task = 0;
    start_first_task(tasks[0].esp);
}

uint32_t scheduler_tick(uint32_t old_esp)
{
    if (task_count == 0) return old_esp;

    global_tick++;

    int prev = current_task;
    tasks[current_task].esp = old_esp;

    for (int i = 0; i < task_count; i++)
    {
        if (tasks[i].state == TASK_SLEEPING && global_tick >= tasks[i].wake_tick)
        {
            tasks[i].state = TASK_READY;
        }
    }

    // 回收
    for (int i = 0; i < task_count; i++)
    {
        if (i == prev) continue;
        if (tasks[i].state == TASK_END)
        {
            if (tasks[i].stack_base)
            {
                kfree((void *)tasks[i].stack_base);
                tasks[i].stack_base = 0;
            }
            tasks[i].state = TASK_UNUSED;
        }
    }

    int next = (prev + 1) % task_count;
    while (next != prev)
    {
        if (tasks[next].state == TASK_READY || tasks[next].state == TASK_RUNNING)
        {
            break;
        }
        next = (next + 1) % task_count;
    }
    if (next == prev)
    {
        if (tasks[prev].state == TASK_RUNNING || tasks[prev].state == TASK_READY)
        {
            return old_esp;
        }
        return old_esp;
    }
    if (tasks[prev].state == TASK_RUNNING)
    {
        tasks[prev].state = TASK_READY;
    }
    tasks[next].state = TASK_RUNNING;
    current_task = next;

    return tasks[next].esp;
}

void task_exit()
{
    tasks[current_task].state = TASK_END;
    while (1)
    {
        asm volatile("hlt");
    }
}

void task_sleep(uint32_t ms)
{
    int self = current_task;
    uint32_t ticks = (ms + TICK_MS - 1) / TICK_MS;
    tasks[self].wake_tick = global_tick + ticks;
    tasks[self].state = TASK_SLEEPING;

    while (tasks[self].state == TASK_SLEEPING)
    {
        asm volatile("hlt");
    }
}

void scheduler_ps()
{
    print_line("PID    NAME                    STATE");
    for (int i = 0; i < task_count; i++) // ++i 在 类或类的迭代器优于 i++ (C++)
    {
        if (tasks[i].state == TASK_UNUSED) continue;
        print_dec(tasks[i].pid);
        print_string("      ");
        print_string(tasks[i].name);
        print_string("  ");
        if (tasks[i].state == TASK_RUNNING) print_line("running");
        else if (tasks[i].state == TASK_READY) print_line("ready");
        else if (tasks[i].state == TASK_END)     print_line("dead");
        else if (tasks[i].state == TASK_SLEEPING) print_line("sleeping");
        else print_line("unused");
    }
}

int scheduler_kill(int pid)
{
    for (int i = 0; i < task_count; i++)
    {
        if (tasks[i].pid == pid && tasks[i].state != TASK_UNUSED)
        {
            tasks[i].state = TASK_END;
            return 0;
        }
    }
    return -1;
}