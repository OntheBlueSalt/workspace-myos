#include "idt.h"
#include "../drivers/screen.h"
#include "../shell/shell.h"
#include <stdint.h>


// 定义 IDT 表，共 256 个条目
struct idt_entry idt[256];
struct idt_ptr idt_ptr;


// 外部声明：汇编中的中断入口
extern void keyboard_handler_entry();
extern void timer_handler_entry();


void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}


// 设置 IDT 条目
void idt_set_gate(int num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}


void pic_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
//    outb(0x21, 0xFD);  // 只允许 IRQ1 (键盘)
//    outb(0xA1, 0xFF);  // 屏蔽从片所有中断
    outb(0x21, 0xFC);  // 允许 IRQ0 (定时器) 和 IRQ1 (键盘)
    outb(0xA1, 0xFF);  // 从片全部屏蔽

}


void idt_init() {
    pic_remap();
    // 设置 IDT 指针
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt;

    // 设置键盘中断（向量 33，IRQ1） 和 定时器中断 (32,IRQ0)
    idt_set_gate(32, (uint32_t)timer_handler_entry, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)keyboard_handler_entry, 0x08, 0x8E);

    // 加载 IDT
    asm volatile("lidt %0" : : "m"(idt_ptr));

    // 开启中断
    asm volatile("sti");
}


// 键盘中断处理函数（C 部分）
void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    static int e0_prefix = 0;    // 静态调用

    if (scancode == 0xE0) {
        e0_prefix = 1;
        outb(0x20, 0x20);   // 发送EOI
        return;
    }

    // 移动
    if (e0_prefix) {
        e0_prefix = 0;
        if (scancode == 0x4B) {
            move_cursor_left();
        } else if (scancode == 0x4D) {
            move_cursor_right();
        }
        outb(0x20, 0x20);
        return;
    }

    // 普通键处理
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    // 退格键
    if (scancode == 0x0E) {
        shell_input_char('\b');
        outb(0x20, 0x20);
        return;
    }

    // 回车键
    if (scancode == 0x1C) {
        shell_input_char('\n');
        outb(0x20, 0x20);
        return;
    }

    char c = '?';

    switch (scancode) {
        case 0x02: c = '1'; break;
        case 0x03: c = '2'; break;
        case 0x04: c = '3'; break;
        case 0x05: c = '4'; break;
        case 0x06: c = '5'; break;
        case 0x07: c = '6'; break;
        case 0x08: c = '7'; break;
        case 0x09: c = '8'; break;
        case 0x0A: c = '9'; break;
        case 0x0B: c = '0'; break;
        case 0x10: c = 'q'; break;
        case 0x11: c = 'w'; break;
        case 0x12: c = 'e'; break;
        case 0x13: c = 'r'; break;
        case 0x14: c = 't'; break;
        case 0x15: c = 'y'; break;
        case 0x16: c = 'u'; break;
        case 0x17: c = 'i'; break;
        case 0x18: c = 'o'; break;
        case 0x19: c = 'p'; break;
        case 0x1E: c = 'a'; break;
        case 0x1F: c = 's'; break;
        case 0x20: c = 'd'; break;
        case 0x21: c = 'f'; break;
        case 0x22: c = 'g'; break;
        case 0x23: c = 'h'; break;
        case 0x24: c = 'j'; break;
        case 0x25: c = 'k'; break;
        case 0x26: c = 'l'; break;
        case 0x2C: c = 'z'; break;
        case 0x2D: c = 'x'; break;
        case 0x2E: c = 'c'; break;
        case 0x2F: c = 'v'; break;
        case 0x30: c = 'b'; break;
        case 0x31: c = 'n'; break;
        case 0x32: c = 'm'; break;
        case 0x39: c = ' '; break;
        default: c = 0; break;
    }
    if (c != 0) {
        shell_input_char(c);
    }

    // 发送 EOI
    outb(0x20, 0x20);
}

// int tick_count = 0;

void timer_handler() {
    /*tick_count++;

    int row = 24;
    int col = 70;

    char *video = (char *) 0xB8000;
    int offset = (row * 80 + col) * 2;

    char buf[10];
    int n = tick_count;
    int i = 0;
    if (n == 0) buf[i++] = '0';
    while (n>0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    // 逆序
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }
    buf[i] = '\0';

    for (int j = 0; j < i; j++) {
        video[(row * 80 + col + j) * 2] = buf[j];
        video[(row * 80 + col + j) * 2 + 1] = 0x0F;
    }
    */

//    scheduler_tick();
    outb(0x20, 0x20);
}