#include "scheduler.h"
#include "../mm/heap.h"
#include "../drivers/screen.h"
#include "../lib/string.h"

static task_t tasks[MAX_TASKS];
static int task_count = 0;
static int current_task = 0;

#define STACK_SIZE 2048

extern void start_first_task(uint32_t esp);

static void setup_task_stack(task_t *t, void (*entry)())
{
    uint32_t *stack = kmalloc(STACK_SIZE);
    if (!stack) return;

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
        tasks[i].name[0] = '\0';
    }
    task_count = 0;
    current_task = 0;
}

int scheduler_create_task(const char *name, void (*entry)())
{
    if (task_count >= MAX_TASKS) return -1;
    int id = task_count;

    tasks[id].pid = id + 1;
    str_copy(tasks[id].name, name, TASK_NAME_MAX);
    tasks[id].state = TASK_READY;

    setup_task_stack(&tasks[id], entry);

    if (tasks[id].esp == 0) return -1;
    task_count++;
    return tasks[id].pid;
}

void scheduler_start()
{
    if (task_count == 0) return;
    asm volatile("sti");
    tasks[0].state = TASK_RUNNING;
    current_task = 0;
    start_first_task(tasks[0].esp);
}

uint32_t scheduler_tick(uint32_t old_esp)
{
    if (task_count == 0) return old_esp;

    tasks[current_task].esp = old_esp;

    int next = (current_task + 1) % task_count;
    while (next != current_task && tasks[next].state == TASK_UNUSED)
    {
        next = (next + 1) % task_count;
    }
    if (next == current_task)
    {
        return old_esp;
    }
    if (tasks[current_task].state == TASK_RUNNING)
    {
        tasks[current_task].state = TASK_READY;
    }
    current_task = next;

    return tasks[next].esp;
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
        else print_line("unused");
    }
}

int scheduler_kill(int pid)
{
    for (int i = 0; i < task_count; i++)
    {
        if (tasks[i].pid == pid && tasks[i].state != TASK_UNUSED)
        {
            tasks[i].state = TASK_UNUSED;
            return 0;
        }
    }
    return -1;
}