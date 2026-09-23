#include "paging.h"
#include "../drivers/screen.h"

// 页目录和页表必须4KB对齐
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_tables[4][1024] __attribute__((aligned(4096)));

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2

void paging_init()
{
    // 建立4个页表 恒等映射前 16MB
    for (int t = 0; t < 4; ++t)
    {
        for (int i = 0; i < 1024; ++i)
        {
            uint32_t phys = (t * 1024 + i) * 4096;
            page_tables[t][i] = phys | PAGE_PRESENT | PAGE_RW;
        }
        page_directory[t] = ((uint32_t)page_tables[t]) | PAGE_PRESENT | PAGE_RW;
    }
    // 其余页目项填 0 (未映射)
    for (int i = 4; i < 1024; ++i)
    {
        page_directory[i] = 0;
    }

    // 加载页目录到 CR3
    asm volatile ("mov %0, %%cr3" : : "r" (page_directory));

    // 设置CR0 的 PG 位(第31位) 开启分页
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" : : "r" (cr0));

    print_line("Paging enabled");
}