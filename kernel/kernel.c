#include "screen.h"

void main() {
    clear_screen();
    print_line("Hello from C kernel!");
    print_line("This is my OS.");
    while (1) {}
}