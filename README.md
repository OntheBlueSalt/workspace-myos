# workspace-myos

一个从零开始编写的 32 位 x86 操作系统，运行在 QEMU 上。

## 功能

- **引导**：BIOS → 引导扇区 → 保护模式 → C 内核
- **中断**：IDT、PIC 重映射、键盘中断、定时器中断
- **键盘**：Shift/Ctrl 修饰键、符号、方向键、Tab 补全、Ctrl+C/L
- **屏幕**：文本模式输出、光标控制、自动滚动
- **内存**：物理页分配（位图法）、内核堆分配（`kmalloc` / `kfree`）
- **文件系统**：内存目录树（ramfs），支持文件/目录、绝对/相对路径
- **Shell**：命令历史、Tab 补全、当前工作目录（`cwd`）
- **多任务**：抢占式调度（定时器时间片）、任务创建/查看/终止

## 目录结构

```text
workspace-myos/
├── boot/
│   └── boot.asm              # 引导扇区
├── kernel/
│   ├── arch/                 # 架构相关（汇编）
│   │   ├── start.asm         # 内核入口
│   │   ├── switch.asm        # 任务上下文切换
│   │   └── interrupt.asm     # 中断入口
│   ├── drivers/              # 设备驱动
│   │   ├── screen.c
│   │   └── screen.h
│   ├── fs/                   # 内存文件系统
│   │   ├── ramfs.c
│   │   └── ramfs.h
│   ├── interrupt/            # 中断管理
│   │   ├── idt.c
│   │   └── idt.h
│   ├── lib/                  # 通用工具
│   │   ├── stdint.h          # 自制标准类型
│   │   ├── string.c
│   │   └── string.h
│   ├── mm/                   # 内存管理
│   │   ├── memory.c
│   │   ├── memory.h
│   │   ├── heap.c
│   │   └── heap.h
│   ├── sched/                # 任务调度
│   │   ├── scheduler.c
│   │   ├── scheduler.h
│   │   ├── tasks.c           # 示例任务
│   │   └── tasks.h
│   ├── shell/                # shell
│   │   ├── shell.c
│   │   └── shell.h
│   ├── kernel.c              # 内核主入口
│   └── linker.ld             # 链接脚本
├── build/                    # 编译产物（gitignore）
├── .gitignore
├── Makefile
└── README.md
```

## evn
wsl2 Ubuntn 24环境下进行

## 准备工作：
```bash
sudo apt update 
sudo apt install build-essential nasm qemu-system-x86 gdb
```
- nasm:汇编器
- gcc:C编辑器
- qemu-system-x86:模拟x86计算机
- gdb:调试器

## 构建与运行
```bash
make clean # 清理
make run #运行
```

## 相关命令
```bash
# 查看ELF内核入口
readelf -h build/kernel.elf | grep Entry
# 查看引导程序跳转地址 找出call xx  xx的地址和上面的要保持一致
cat boot/boot.ams | head -100
# 查看内核二进制起始内容 确认第一条地址是否一致
objdump -d build/kernel.elf | head -20
# 检查链接脚本
cat kernel/linker.ld | head -100
```



