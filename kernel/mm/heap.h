#ifndef HEAP_H
#define HEAP_H

#include "../lib/stdint.h"

void  heap_init();
void* kmalloc(uint32_t size);
void  kfree(void* ptr);
uint32_t heap_free_bytes();

#endif