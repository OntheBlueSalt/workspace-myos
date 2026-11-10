#ifndef GDT_H
#define GDT_H

#include "../lib/stdint.h"

/* 段选择子（RPL=0） */
#define KERNEL_CODE_SEL  0x08
#define KERNEL_DATA_SEL  0x10
#define USER_CODE_SEL    0x18   /* Ring3 使用时加 RPL=3 -> 0x1B */
#define USER_DATA_SEL    0x20   /* Ring3 使用时加 RPL=3 -> 0x23 */
#define TSS_SEL          0x28

void gdt_init(void);
void tss_set_esp0(uint32_t esp0);

#endif
