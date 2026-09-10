#include "heap.h"
#include "memory.h"
#include "../drivers/screen.h"

#define BLOCK_SIZE  32          // 每个块 32 字节
#define BLOCKS_PER_PAGE (PAGE_SIZE / BLOCK_SIZE)    // 128 块/页

#define MAX_HEAP_PAGES  16

static uint8_t  block_bitmap[MAX_HEAP_PAGES][BLOCKS_PER_PAGE / 8];
static void*    page_addrs[MAX_HEAP_PAGES];
static int      page_count = 0;
static uint32_t free_blocks = 0;

static void set_block_used(int page, int block) {
    block_bitmap[page][block / 8] |= (1 << block % 8);
}

static void set_block_free(int page, int block) {
    block_bitmap[page][block / 8] |= ~(1 << block % 8);
}

static int is_block_used (int page, int block) {
    return block_bitmap[page][block / 8] & (1 << (block % 8));
}

void heap_init() {
    page_count = 0;
    free_blocks = 0;
    for (int i = 0; i < MAX_HEAP_PAGES; i++) {
        for (int j = 0; j < BLOCKS_PER_PAGE / 8; j++) {
            block_bitmap[i][j] = 0;
        }
    }
}

void* kmalloc(uint32_t size) {
    if (size == 0 || size > BLOCK_SIZE) return 0;

    // 在现有页里找空闲块
    for (int p = 0; p < page_count; p++) {
        for (int b = 0; b < BLOCKS_PER_PAGE; b++) {
            if (!is_block_used(p, b)) {
                set_block_used(p, b);
                free_blocks--;
                return (void*)((uint32_t)page_addrs[p] + b * BLOCK_SIZE);
            }
        }
    }

    // 没有空闲块，分配新页
    if (page_count >= MAX_HEAP_PAGES) return 0;
    void* page = alloc_page();
    if (!page) return 0;

    page_addrs[page_count] = page;
    // 清空新页的位图
    for (int j = 0; j < BLOCKS_PER_PAGE / 8; j++) {
        block_bitmap[page_count][j] = 0;
    }
    free_blocks += BLOCKS_PER_PAGE;

    // 标记第一个块已使用并返回
    int p = page_count;
    page_count++;
    set_block_used(p, 0);
    free_blocks--;
    return (void*)((uint32_t)page + 0);
}

void kfree(void* ptr) {
    if (!ptr) return;
    uint32_t addr = (uint32_t)ptr;

    for (int p = 0; p < page_count; p++) {
        uint32_t base = (uint32_t)page_addrs[p];
        if (addr >= base && addr < base + PAGE_SIZE) {
            int b = (addr - base) / BLOCK_SIZE;
            if (is_block_used(p, b)) {
                set_block_free(p, b);
                free_blocks++;
            }
            return;
        }
    }
}

uint32_t heap_free_bytes() {
    return free_blocks * BLOCK_SIZE;
}

