#include "screen.h"
#include "idt.h"

void main() {

    clear_screen();
    print_line("Hello from C kernel!");
    print_line("This is my OS...");

    idt_init();     // 中断处理

    while (1) {
        asm volatile("hlt");
    }
}