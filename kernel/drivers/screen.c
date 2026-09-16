#include "screen.h"
#include "../lib/stdint.h"

extern void outb(uint16_t port, uint8_t value);

#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

static int cursor_row = 0;
static int cursor_col = 0;

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

// 滚轮滑动
static void scroll_up()
{
    char *video = (char *) VIDEO_MEMORY;
    for (int i = 0; i < (MAX_ROWS - 1) * MAX_COLS; i++)
    {
        video[i * 2] = video[(i + MAX_COLS) * 2];
        video[i * 2 + 1] = video[(i + MAX_COLS) * 2 + 1];
    }

    for (int i = (MAX_ROWS - 1) * MAX_COLS; i < MAX_COLS * MAX_ROWS; i++)
    {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x0F;
    }
}

void print_char(char c) {
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
        if (cursor_row >= MAX_ROWS) {
            scroll_up();
            cursor_row = MAX_ROWS - 1;
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
            scroll_up();
            cursor_row = MAX_ROWS - 1;
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

void move_cursor_left() {
    if (cursor_col > 0) {
        cursor_col--;
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = MAX_COLS - 1;
    }
    update_cursor(cursor_row, cursor_col);
}

void move_cursor_right() {
    if (cursor_col < MAX_COLS - 1) {
        cursor_col++;
    } else if (cursor_row < MAX_ROWS - 1) {
        cursor_row++;
        cursor_col = 0;
    }
    update_cursor(cursor_row, cursor_col);
}

void backspace() {
    if (cursor_col > 0) {
        cursor_col--;
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = MAX_COLS - 1;
    }
    // 用空格覆盖当前位置
    char *video = (char *) VIDEO_MEMORY;
    int offset = (cursor_row * MAX_COLS + cursor_col) * 2;
    video[offset] = ' ';
    video[offset + 1] = 0x0F;
    update_cursor(cursor_row, cursor_col);
}

void print_dec(uint32_t num) {
    if (num == 0) {
        print_char('0');
        return;
    }
    char buf[12];
    int i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num = num / 10;   // 正确赋值
    }
    for (int j = i - 1; j >= 0; j--) {
        print_char(buf[j]);
    }
}