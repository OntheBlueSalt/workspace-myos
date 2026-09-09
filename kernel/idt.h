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

// 初始化
void idt_init();

#endif
