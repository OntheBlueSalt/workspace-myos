#include "screen.h"
#include <stdint.h>

#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

static int cursor_row = 0;
static int cursor_col = 0;

// 端口输出字节
void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// 更新硬件光标位置
void update_cursor(int row, int col) {
    uint16_t pos = row * MAX_COLS + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void clear_screen() {
    char *video = (char *) VIDEO_MEMORY;
    for (int i = 0; i < MAX_ROWS * MAX_COLS; i++) {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x0F;
    }
    cursor_row = 0;
    cursor_col = 0;
    update_cursor(0, 0);
}

void print_char(char c) {
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
        if (cursor_row >= MAX_ROWS) {
            cursor_row = 0;   // 简单滚动，回到顶部
        }
        update_cursor(cursor_row, cursor_col);
        return;
    }
    char *video = (char *) VIDEO_MEMORY;
    int offset = (cursor_row * MAX_COLS + cursor_col) * 2;
    video[offset] = c;
    video[offset + 1] = 0x0F;
    cursor_col++;
    if (cursor_col >= MAX_COLS) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= MAX_ROWS) {
            cursor_row = 0;
        }
    }
    update_cursor(cursor_row, cursor_col);
}

void print_string(const char *str) {
    while (*str != '\0') {
        print_char(*str);
        str++;
    }
}

void print_line(const char *str) {
    print_string(str);
    print_char('\n');
}