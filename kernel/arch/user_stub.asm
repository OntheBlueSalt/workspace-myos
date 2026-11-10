[bits 32]
section .text

global user_stub_start
global user_stub_end

; 这段代码会被复制到 0x40000000 执行，必须与位置无关
user_stub_start:
.loop:
    ; 在 24 行 60 列打印 'U'
    mov eax, 1          ; SYS_PRINT
    mov ebx, 'U'
    mov ecx, 60
    int 0x80

    ; 打印当前 CPL：CS & 3，期望 '3'
    mov eax, cs
    and eax, 3
    add eax, '0'
    mov ebx, eax
    mov eax, 1          ; SYS_PRINT
    mov ecx, 61
    int 0x80

    ; 忙等，让定时器有机会切换任务
    mov edx, 0x800000
.delay:
    dec edx
    jnz .delay
    jmp .loop

user_stub_end: