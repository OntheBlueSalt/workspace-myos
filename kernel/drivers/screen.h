#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

void print_char(char c);
void print_string(const char *str);
void print_line(const char *str);   // 打印字符串并换行
void clear_screen();
void update_cursor(int row, int col);   // 更新光标位置
void move_cursor_left();
void move_cursor_right();
void backspace();
void print_dec(uint32_t num);

#endif