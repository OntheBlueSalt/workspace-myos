#include "exception.h"
#include "idt.h"
#include "../drivers/screen.h"

extern void isr0(void);  extern void isr1(void);  extern void isr2(void);  extern void isr3(void);
extern void isr4(void);  extern void isr5(void);  extern void isr6(void);  extern void isr7(void);
extern void isr8(void);  extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

static void (*const exception_stubs[32])(void) = {
    isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
    isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
    isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
    isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

static const char *const exception_names[32] = {
    "Divide Error",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

static void print_hex(uint32_t value)
{
    char buf[9];
    buf[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        uint32_t digit = value & 0xF;
        buf[i] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value >>= 4;
    }

    print_string("0x");
    print_string(buf);
}

void exception_dispatch(exception_frame_t *frame)
{
    uint32_t cr2 = 0;

    if (frame->vector == 14) {
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
    }

    print_line("");
    print_line("*** CPU EXCEPTION ***");

    print_string("Name: ");
    if (frame->vector < 32) {
        print_line(exception_names[frame->vector]);
    } else {
        print_line("Unknown");
    }

    print_string("Vector: ");
    print_dec(frame->vector);
    print_char('\n');

    print_string("Error: ");
    print_hex(frame->error_code);
    print_char('\n');

    print_string("EIP: ");
    print_hex(frame->eip);
    print_string("  CS: ");
    print_hex(frame->cs);
    print_char('\n');

    print_string("EFLAGS: ");
    print_hex(frame->eflags);
    print_char('\n');

    if (frame->vector == 14) {
        print_string("CR2: ");
        print_hex(cr2);
        print_char('\n');
    }

    print_line("System halted.");

    for (;;) {
        asm volatile("cli; hlt");
    }
}

void exception_init(void)
{
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)exception_stubs[i], 0x08, 0x8E);
    }
}