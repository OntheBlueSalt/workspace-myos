[bits 32]
section .text

global load_idt
global keyboard_handler_entry
global timer_handler_entry
global syscall_entry

extern keyboard_handler
extern timer_handler
extern syscall_dispatch

load_idt:
    mov eax, [esp + 4]    ; 参数：idt_ptr 结构体指针
    lidt [eax]
    ret

keyboard_handler_entry:
    pusha                 ; 保存所有通用寄存器
    call keyboard_handler ; 调用 C 处理函数
    popa                  ; 恢复寄存器
    iretd                 ; 中断返回

timer_handler_entry:
    pusha                    ; 保存通用寄存器
    push esp                 ; 参数：old_esp（指向保存的通用寄存器）
    call timer_handler       ; 返回新任务 esp 在 EAX
    mov esp, eax             ; 切换到新任务栈
    popa                     ; 恢复新任务的通用寄存器
    iretd                    ; 弹出 EIP/CS/EFLAGS，返回新任务

syscall_entry:
    pusha
    push edx            ; 参数 c
    push ecx            ; 参数 b
    push ebx            ; 参数 a
    push eax            ; 系统调用号
    call syscall_dispatch
    add esp, 16         ; 4 个参数
    mov [esp + 28], eax
    popa
    iretd

extern exception_dispatch

%macro ISR_NOERR 1
global isr%1
isr%1:
    push dword 0
    push dword %1
    jmp exception_common
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    push dword %1
    jmp exception_common
%endmacro

exception_common:
    pusha

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    cld

    mov eax, esp
    push eax
    call exception_dispatch
    add esp, 4

    cli
.hang:
    hlt
    jmp .hang

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30
ISR_NOERR 31