#include "screen.h"
#include "idt.h"
#include "scheduler.h"

void main() {

    clear_screen();
    print_line("Hello from C kernel!");
    print_line("This is my OS...");
    print_char('\n');

    idt_init();     // 中断处理
    scheduler_init();
    scheduler_start();

    while (1) {
        asm volatile("hlt");
    }
}