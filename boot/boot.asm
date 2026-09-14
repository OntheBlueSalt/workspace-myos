; 引导扇区
[org 0x7c00]
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000        ; 设置栈

    mov [BOOT_DRIVE], dl   ; 保存启动驱动器号

    mov bx, MSG_LOAD_KERNEL
    call print_string

    call load_kernel       ; 读取内核

    call switch_to_pm      ; 切换到保护模式

    jmp $

; ---------- 打印字符串（实模式） ----------
print_string:
    pusha
    mov ah, 0x0e
.loop:
    mov al, [bx]
    cmp al, 0
    je .done
    int 0x10
    inc bx
    jmp .loop
.done:
    popa
    ret

; ---------- 读取内核到 0x10000 ----------
load_kernel:
    pusha
    push es
    mov bx, 0x1000         ; ES:BX = 0x1000:0x0000 => 物理地址 0x10000
    mov es, bx
    mov bx, 0x0000

    mov ah, 0x02           ; BIOS 读扇区
    mov al, 64             ; 读取 64 个扇区/32KB
    mov ch, 0x00           ; 柱面 0
    mov cl, 0x02           ; 从扇区 2 开始
    mov dh, 0x00           ; 磁头 0
    mov dl, [BOOT_DRIVE]   ; 驱动器号
    int 0x13
    jc disk_error
    pop es
    popa
    ret

disk_error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

; ---------- 切换到保护模式 ----------
switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    ; 跳转到 C 内核
    call 0x10000
    jmp $

; ---------- GDT ----------
gdt_start:
    dq 0x0
gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

BOOT_DRIVE db 0
MSG_LOAD_KERNEL db "Loading kernel...", 0
DISK_ERROR_MSG db "Disk read error!", 0

times 510-($-$$) db 0
dw 0xaa55