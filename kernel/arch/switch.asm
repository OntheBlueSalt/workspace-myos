[bits 32]
section .text

global switch_to
global start_first_task

start_first_task:
    mov eax, [esp + 4]   ; 参数：任务esp
    mov esp, eax
    popa
    ret
; void switch_to(uint32_t *old_esp, uint32_t new_esp)
; 参数通过栈传递：old_esp 在 [esp+4]，new_esp 在 [esp+8]
switch_to:
    ; 进入时栈: [esp] = 返回地址, [esp+4] = old_esp 指针, [esp+8] = new_esp 值
    mov eax, [esp + 4]      ; 获取 old_esp 指针
    mov ecx, [esp + 8]      ; 获取 new_esp 值
    pusha                   ; 保存所有通用寄存器
    mov [eax], esp          ; 将当前 esp 保存到 old_esp 指向的位置
    mov esp, ecx            ; 切换到新任务栈
    popa                    ; 恢复新任务的通用寄存器
    ret                     ; 返回到新任务