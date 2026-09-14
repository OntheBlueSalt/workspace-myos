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
    static int e0_prefix = 0;
    static int shift_pressed = 0;
    static int ctrl_pressed = 0;

    static const char keymap_normal[128] = {
        0,   27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
        0,   '\\','z','x','c','v','b','n','m',',','.','/',
        0,   '*', 0,  ' ',
    };

    static const char keymap_shift[128] = {
        0,   27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
        '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
        0,   'A','S','D','F','G','H','J','K','L',':','"','~',
        0,   '|','Z','X','C','V','B','N','M','<','>','?',
        0,   '*', 0,  ' ',
    };

    // 处理 0xE0 前缀（方向键）
    if (scancode == 0xE0) {
        e0_prefix = 1;
        outb(0x20, 0x20);
        return;
    }
    if (e0_prefix) {
        e0_prefix = 0;
        if (scancode == 0x4B) move_cursor_left();
        else if (scancode == 0x4D) move_cursor_right();
        else if (scancode == 0x48) shell_history_up();
        else if (scancode == 0x50) shell_history_down();
        outb(0x20, 0x20);
        return;
    }

    // Shift 按下
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        outb(0x20, 0x20);
        return;
    }
    // Shift 释放
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        outb(0x20, 0x20);
        return;
    }
    // Ctrl 按下
    if (scancode == 0x1D) {
        ctrl_pressed = 1;
        outb(0x20, 0x20);
        return;
    }
    // Ctrl 释放
    if (scancode == 0x9D) {
        ctrl_pressed = 0;
        outb(0x20, 0x20);
        return;
    }

    // 其他键：只处理按下事件（最高位 0）
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    // 退格
    if (scancode == 0x0E) {
        shell_input_char('\b');
        outb(0x20, 0x20);
        return;
    }
    // tab
    if (scancode == 0x0F) {
        shell_tab_complete();
        outb(0x20, 0x20);
        return;
    }
    // 回车
    if (scancode == 0x1C) {
        shell_input_char('\n');
        outb(0x20, 0x20);
        return;
    }
    // 查表
    char c = 0;
    if (scancode < 128) {
        c = shift_pressed ? keymap_shift[scancode]
                          : keymap_normal[scancode];
        if (ctrl_pressed && c >= 'a' && c <= 'z') {
            c = c - 'a' + 1;
        }
    }
    if (c != 0) {
        shell_input_char(c);
    }

    outb(0x20, 0x20);
}

void timer_handler() {
    outb(0x20, 0x20);
}