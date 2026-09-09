#include "idt.h"
#include "screen.h"
#include <stdint.h>


// 定义 IDT 表，共 256 个条目
struct idt_entry idt[256];
struct idt_ptr idt_ptr;


// 外部声明：汇编中的中断入口
extern void keyboard_handler_entry();


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
    outb(0x21, 0xFD);  // 主片：只允许 IRQ1 (键盘)，屏蔽 IRQ0 (定时器) 等其他中断
    outb(0xA1, 0xFF);  // 从片：屏蔽所有中断    
}


void idt_init() {
    pic_remap();
    // 设置 IDT 指针
    idt_ptr.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt;

    // 设置键盘中断（向量 33，IRQ1）
    idt_set_gate(33, (uint32_t)keyboard_handler_entry, 0x08, 0x8E);

    // 加载 IDT
    asm volatile("lidt %0" : : "m"(idt_ptr));

    // 开启中断
    asm volatile("sti");
}


// 键盘中断处理函数（C 部分）
void keyboard_handler() {
    uint8_t scancode = inb(0x60);

    if (!(scancode & 0x80)) {  // 按下事件
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
            print_char(c);
        }
    }

    // 发送 EOI
    outb(0x20, 0x20);
}