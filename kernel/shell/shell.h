#ifndef SHELL_H
#define SHELL_H

#include "../lib/stdint.h"

void shell_input_char(char c);

void shell_run();

void shell_history_up();
void shell_history_down();

void shell_tab_complete();

#endif