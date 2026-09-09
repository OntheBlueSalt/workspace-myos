[bits 32]
section .text

global load_idt
global keyboard_handler_entry

extern keyboard_handler

load_idt:
    mov eax, [esp + 4]    ; 参数：idt_ptr 结构体指针
    lidt [eax]
    ret

keyboard_handler_entry:
    pusha                 ; 保存所有通用寄存器
    call keyboard_handler ; 调用 C 处理函数
    popa                  ; 恢复寄存器
    iretd                 ; 中断返回