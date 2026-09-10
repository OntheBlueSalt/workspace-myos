#include "memory.h"
#include "../drivers/screen.h"

static uint8_t bitmap[BITMAP_SIZE];
static uint32_t free_pages = 0;

static void set_bit(uint32_t page) {
    bitmap[page / 8] |= (1 << (page % 8));
}

static void clear_bit(uint32_t page) {
    bitmap[page / 8] |= ~(1 << (page % 8));
}

static int test_bit(uint32_t page) {
    return bitmap[page / 8] & (1 << (page % 8));
}

void memory_init() {
    for (int i = 0; i < BITMAP_SIZE; ++i) {
        bitmap[i] = 0xFF;
    }
    free_pages = 0;

    for (uint32_t page = 256; page < TOTAL_PAGES; page++) {
        clear_bit(page);
        free_pages++;
    }
}

void * alloc_page() {
    if (free_pages == 0) return 0;

    for (uint32_t page = 256; page < TOTAL_PAGES; page++) {
        if (!test_bit(page)) {
            set_bit(page);
            free_pages--;
            return (void*)(page * PAGE_SIZE);
        }
    }
    return 0;
}

void free_page(void* addr) {
    uint32_t page = (uint32_t)addr / PAGE_SIZE;
    if (page >= 256 && page < TOTAL_PAGES && test_bit(page)) {
        clear_bit(page);
        free_pages++;
    }
}

uint32_t get_free_page_count() {
    return free_pages;
}
