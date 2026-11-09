#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "../lib/stdint.h"

typedef struct
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t vector;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} exception_frame_t;

void exception_init(void);
void exception_dispatch(exception_frame_t *frame);

#endif
