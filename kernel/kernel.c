#include "drivers/screen.h"
#include "fs/ramfs.h"
#include "interrupt/idt.h"
#include "mm/memory.h"
#include "mm/heap.h"
#include "mm/paging.h"
#include "shell/shell.h"
#include "sched/scheduler.h"
#include "sched/tasks.h"
#include "syscall/syscall.h"
#include "interrupt/exception.h"

extern uint32_t __bss_start;
extern uint32_t __bss_end;

void clear_bss() {
    uint32_t *p = &__bss_start;
    while (p < &__bss_end) {
        *p++ = 0;
    }
}

void main() {
    clear_bss();

    clear_screen();
    print_line("Hello from C kernel!");
    print_line("This is my OS...");

    memory_init();
    heap_init();
    ramfs_init();
    paging_init();
    exception_init();
    idt_init();

    uint32_t test_page = (uint32_t)alloc_page();
    if (!test_page) {
        print_line("user map test: alloc failed");
    } else {
        paging_map_user(0x40003000, test_page, PAGE_PRESENT | PAGE_RW | PAGE_USER);

        volatile uint32_t *test = (volatile uint32_t *)0x40003000;
        *test = 0x12345678;

        if (*test == 0x12345678) {
            print_line("user map ok");
        } else {
            print_line("user map failed");
        }
    }
    
    syscall_init();

    scheduler_init();
    scheduler_create_task("shell", shell_run);   // 任务 0
    scheduler_create_task("A", task_a);          // 任务 1
    scheduler_create_task("B", task_b);          // 任务 2
    scheduler_create_task("C", task_c);          // 任务 3

    scheduler_start();   // 切到 shell，之后定时器中断会自动切换任务

    while (1) {
        asm volatile("hlt");
    }
}