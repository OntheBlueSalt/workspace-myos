#include "heap.h"
#include "memory.h"
#include "../drivers/screen.h"

#define MAX_HEAP_PAGES 16
#define HEADER_SIZE   sizeof(block_header_t)

typedef struct block_header {
    uint32_t size;              // 块大小（含头部）
    uint32_t is_free;           // 1 空闲，0 已用
    struct block_header* next;  // 同一页内下一个块
} block_header_t;

static void*    page_addrs[MAX_HEAP_PAGES];
static int      page_count = 0;

static uint32_t align(uint32_t size) {
    return (size + 3) & ~3;
}

static void init_page(void* page) {
    block_header_t* block = (block_header_t*)page;
    block->size = PAGE_SIZE;
    block->is_free = 1;
    block->next = 0;
}

// 从块 b 中切出 need 字节，返回用户指针
static void* split_and_return(block_header_t* b, uint32_t need) {
    // 若剩余空间足够容纳一个新头部加一个最小块，则分裂
    if (b->size >= need + HEADER_SIZE + 4) {
        block_header_t* new_block = (block_header_t*)((uint32_t)b + need);
        new_block->size = b->size - need;
        new_block->is_free = 1;
        new_block->next = b->next;
        b->next = new_block;
        b->size = need;
    }
    b->is_free = 0;
    return (void*)((uint32_t)b + HEADER_SIZE);
}

void heap_init() {
    page_count = 0;
}

void* kmalloc(uint32_t size) {
    if (size == 0) return 0;
    uint32_t need = align(size) + HEADER_SIZE;

    // 在现有页里查找
    for (int p = 0; p < page_count; p++) {
        block_header_t* b = (block_header_t*)page_addrs[p];
        while (b) {
            if (b->is_free && b->size >= need) {
                return split_and_return(b, need);
            }
            b = b->next;
        }
    }

    // 没有合适的块，分配新页
    if (page_count >= MAX_HEAP_PAGES) return 0;
    void* page = alloc_page();
    if (!page) return 0;

    init_page(page);
    page_addrs[page_count++] = page;

    // 在新页上切分（整页就是一个大空闲块）
    block_header_t* b = (block_header_t*)page;
    return split_and_return(b, need);
}

void kfree(void* ptr) {
    if (!ptr) return;
    uint32_t addr = (uint32_t)ptr - HEADER_SIZE;

    for (int p = 0; p < page_count; p++) {
        uint32_t base = (uint32_t)page_addrs[p];
        if (addr >= base && addr < base + PAGE_SIZE) {
            block_header_t* b = (block_header_t*)addr;
            if (b->is_free) return;
            b->is_free = 1;

            // 合并相邻空闲块
            block_header_t* cur = (block_header_t*)base;
            while (cur && cur->next) {
                if (cur->is_free && cur->next->is_free) {
                    cur->size += cur->next->size;
                    cur->next = cur->next->next;
                } else {
                    cur = cur->next;
                }
            }
            return;
        }
    }
}

uint32_t heap_free_bytes() {
    uint32_t total = 0;
    for (int p = 0; p < page_count; p++) {
        block_header_t* b = (block_header_t*)page_addrs[p];
        while (b) {
            if (b->is_free) total += b->size;
            b = b->next;
        }
    }
    return total;
}