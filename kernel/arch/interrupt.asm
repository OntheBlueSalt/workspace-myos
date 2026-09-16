[bits 32]
section .text

global load_idt
global keyboard_handler_entry
global timer_handler_entry

extern keyboard_handler
extern timer_handler

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