[bits 32]
section .text

global gdt_flush
global tss_flush

; void gdt_flush(uint32_t gdt_ptr_addr)
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.reload_cs
.reload_cs:
    ret

; void tss_flush(void)
tss_flush:
    mov ax, 0x28
    ltr ax
    ret