#ifndef SYSCALL_H
#define SYSCALL_H

#include "../lib/stdint.h"

#define SYS_PRINT    1
#define SYS_EXIT     2
#define SYS_SLEEP    3
#define SYS_GETPID   4

void syscall_init();

// 供任务调用的封装
void sys_print(char c);
void sys_exit();
void sys_sleep(uint32_t ms);
int sys_getpid();

#endif