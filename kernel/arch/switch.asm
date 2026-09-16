[bits 32]
section .text

global switch_to
global start_first_task

; void start_first_task(uint32_t esp)
start_first_task:
    mov eax, [esp + 4]   ; 参数：任务esp
    mov esp, eax
    popa
    iretd