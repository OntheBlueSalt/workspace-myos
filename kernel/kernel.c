#include "drivers/screen.h"
#include "interrupt/idt.h"
#include "sched/scheduler.h"
#include "mm/memory.h"

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
    print_string("Free pages: ");
    print_dec(get_free_page_count());
    print_char('\n');

    void* p1 = alloc_page();
    void* p2 = alloc_page();
    void* p3 = alloc_page();

    print_string("After 3 allocs: ");
    print_dec(get_free_page_count());
    print_char('\n');

    free_page(p1);
    free_page(p2);
    free_page(p3);

    print_string("After free: ");
    print_dec(get_free_page_count());
    print_char('\n');

    while (1) {
        asm volatile("hlt");
    }
}