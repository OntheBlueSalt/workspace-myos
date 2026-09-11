#include "drivers/screen.h"
#include "interrupt/idt.h"
#include "mm/memory.h"
#include "mm/heap.h"
#include "shell/shell.h"

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
    idt_init();
    
    shell_run();

    while (1) {
        asm volatile("hlt");
    }
}