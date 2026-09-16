#ifndef MEMORY_H
#define MEMORY_H

#include "../lib/stdint.h"

#define PAGE_SIZE    4096
#define TOTAL_PAGES  4096                    // 16MB / 4KB
#define BITMAP_SIZE  (TOTAL_PAGES / 8)

void     memory_init();
void*    alloc_page();
void     free_page(void* addr);
uint32_t get_free_page_count();

#endif