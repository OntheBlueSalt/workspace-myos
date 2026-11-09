#ifndef PAGING_H
#define PAGING_H

#include "../lib/stdint.h"

// 转移宏防止无法读取
#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

void paging_init(void);
void paging_map_user(uint32_t vaddr, uint32_t paddr, uint32_t flags);

#endif