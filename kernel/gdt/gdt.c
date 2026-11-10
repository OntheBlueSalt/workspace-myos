#include "gdt.h"
#include "../drivers/screen.h"

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  gran;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* 32 位 TSS，共 104 字节 */
typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;

static gdt_entry_t gdt[6];
static gdt_ptr_t   gdt_ptr;
static tss_t       tss;

extern void gdt_flush(uint32_t gdt_ptr_addr);
extern void tss_flush(void);

static void gdt_set_gate(int i, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t gran)
{
    gdt[i].base_low  = base & 0xFFFF;
    gdt[i].base_mid  = (base >> 16) & 0xFF;
    gdt[i].base_high = (base >> 24) & 0xFF;
    gdt[i].limit_low = limit & 0xFFFF;
    gdt[i].gran      = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access    = access;
}

void gdt_init(void)
{
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                  /* 0x00 null */
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);      /* 0x08 内核代码 */
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);      /* 0x10 内核数据 */
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF);      /* 0x18 用户代码 DPL=3 */
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF);      /* 0x20 用户数据 DPL=3 */

    /* TSS：type=9（32 位可用 TSS），DPL=0，字节粒度 */
    gdt_set_gate(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);

    tss.ss0        = KERNEL_DATA_SEL;   /* Ring3 中断时内核栈段 */
    tss.esp0       = 0;                 /* B2 再按任务设置 */
    tss.iomap_base = sizeof(tss);       /* >= limit，表示没有 I/O 位图 */

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();

    uint16_t tr;
    asm volatile("str %0" : "=rm"(tr));

    if (tr == TSS_SEL) {
        print_line("GDT/TSS loaded");
    } else {
        print_string("GDT/TSS failed, TR=");
        print_dec(tr);
        print_line("");
    }
}

void tss_set_esp0(uint32_t esp0)
{
    tss.esp0 = esp0;
}