#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT 条目结构
struct idt_entry {
    uint16_t base_low;    // 处理程序地址低16位
    uint16_t selector;    // 代码段选择子
    uint8_t  zero;        // 保留，必须为0
    uint8_t  flags;       // 类型和属性
    uint16_t base_high;   // 处理程序地址高16位
} __attribute__((packed));

// IDT 指针结构
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

uint8_t inb(uint16_t port);
void idt_set_gate(int num, uint32_t base, uint16_t selector, uint8_t flags);
void pic_remap();
// 初始化
void idt_init();
void keyboard_handler();

#endif
