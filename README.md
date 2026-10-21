# workspace-myos

一个从零开始编写的 32 位 x86 操作系统：BIOS 引导扇区加载内核，进入 32 位保护模式后执行 C 内核，在 QEMU 中提供交互式 Shell、物理页/堆内存管理、内存文件系统和抢占式多任务。

> 项目定位是学习用最小内核：不依赖 GRUB/Multiboot，不链接宿主 libc，重点是把启动、中断、内存、文件系统、调度和 Shell 串成一条可运行的链路。

## 功能

- **引导**：BIOS → 引导扇区（`0x7C00`）→ 读取 128 个扇区到 `0x10000` → 平坦 GDT / 保护模式 → `_start` → C 内核 `main`；启动时清空 `.bss`。
- **中断与定时**：IDT、PIC 重映射（IRQ0/IRQ1 → `0x20`/`0x21`）、PIT 100 Hz 定时器、键盘中断、`int 0x80` 系统调用门。
- **键盘**：Scancode Set 1，Shift/Ctrl 修饰键、符号输入、方向键、Tab 补全、Ctrl+C/L 快捷键。
- **屏幕**：VGA 文本模式 80×25（`0xB8000`），CRTC 光标、换行与自动滚屏，提供 `print_char` / `print_string` / `print_line` / `print_dec`。
- **物理内存**：按 16 MiB / 4096 个 4 KiB 页建立位图；0–1 MiB 保留，从 1 MiB 起分配，支持 `alloc_page` / `free_page` / 空闲页统计。
- **内核堆**：按页向物理分配器取页，首次适配 + 块分裂/合并，提供 `kmalloc` / `kfree`；最多 16 页（64 KiB）。
- **分页**：4 个页表恒等映射前 16 MiB（Present + RW），加载 CR3 并置 `CR0.PG`；当前所有任务共享同一地址空间。
- **文件系统**：ramfs 内存目录树，支持文件/目录、父子链、绝对/相对路径和 `.`/`..`；文件内容缓冲 1024 B（最大 1023 B），重启后丢失。
- **Shell**：128 字节行缓冲、8 条命令历史、命令名 Tab 补全、`cwd` 和绝对/相对路径解析；命令见下表。
- **多任务**：最多 8 个任务，2 KiB 内核栈，10 ms 时间片轮转；支持创建、睡眠、退出、终止和 `ps` 查看；启动时创建 `shell` 与 A/B/C 演示任务。
- **系统调用**：`int 0x80`，`eax` 为调用号，`ebx`/`ecx`/`edx` 传参；已接入 `SYS_PRINT`、`SYS_EXIT`、`SYS_SLEEP`、`SYS_GETPID`。

## Shell 命令

| 命令 | 说明 |
| --- | --- |
| `help` | 显示内置命令列表 |
| `mem` | 显示空闲物理页数和堆空闲字节数 |
| `clear` / `clean` | 清屏 |
| `echo [文本]` | 输出文本；无参数时输出空行 |
| `pwd` | 显示当前目录 |
| `ls [路径]` | 列出目录；目录显示为 `[name]`，文件显示大小 |
| `cd [路径]` | 切换目录；无参数时回到根目录 |
| `mkdir <路径>` | 创建目录 |
| `touch <路径>` | 创建空文件 |
| `cat <路径>` | 显示文件内容 |
| `write <路径> <内容>` | 覆盖写入单行内容（路径不含空格，内容最多 1023 B） |
| `rm <路径>` | 删除文件或空目录 |
| `run a`、`run b`、`run c` | 创建 A/B/C 循环演示任务 |
| `run s` | 创建只运行 5 次后退出的短任务 |
| `run sl` | 创建睡眠演示任务 |
| `ps` | 列出任务及状态 |
| `kill <pid>` | 结束指定任务 |
| `start` | 从任务 0 启动调度（实验性；正常启动时已自动调用） |

命令解析基于空格分割和前缀匹配，不支持引号、转义和管道；`write` 的正文是路径之后的整行剩余文本。文件/目录名最长 31 字节，单文件内容最长 1023 字节。

## 目录结构

```text
workspace-myos/
├── boot/
│   └── boot.asm              # 引导扇区：加载内核、建立 GDT、进入保护模式
├── kernel/
│   ├── arch/                 # 架构相关汇编
│   │   ├── start.asm         # 内核入口 _start
│   │   ├── interrupt.asm     # 中断与系统调用汇编入口
│   │   └── switch.asm        # 首次任务启动 start_first_task
│   ├── drivers/              # 设备驱动
│   │   ├── screen.c          # VGA 文本输出
│   │   └── screen.h
│   ├── fs/                   # 内存文件系统
│   │   ├── ramfs.c
│   │   └── ramfs.h
│   ├── interrupt/            # 中断管理
│   │   ├── idt.c             # IDT / PIC / PIT / 键盘
│   │   └── idt.h
│   ├── lib/                  # 通用工具
│   │   ├── stdint.h          # 固定宽度整数类型
│   │   ├── string.c
│   │   └── string.h
│   ├── mm/                   # 内存管理
│   │   ├── memory.c          # 物理页位图分配
│   │   ├── memory.h
│   │   ├── heap.c            # 内核堆
│   │   ├── heap.h
│   │   ├── paging.c          # 分页初始化
│   │   └── paging.h
│   ├── sched/                # 任务调度
│   │   ├── scheduler.c
│   │   ├── scheduler.h
│   │   ├── tasks.c           # 演示任务
│   │   └── tasks.h
│   ├── shell/                # Shell
│   │   ├── shell.c
│   │   └── shell.h
│   ├── syscall/              # 系统调用
│   │   ├── syscall.c
│   │   └── syscall.h
│   ├── kernel.c              # 内核主入口 main
│   └── linker.ld             # 链接脚本（0x10000）
├── build/                    # 编译产物（gitignore）
├── .gitignore                # Git 忽略规则
├── Makefile
└── README.md
```

## 环境准备

在 WSL2 + Ubuntu 24.04 下验证，其他 Linux 发行版也可使用：

```bash
sudo apt update
sudo apt install -y build-essential nasm qemu-system-x86 gdb
```

- `build-essential`：GCC、GNU ld、make
- `nasm`：汇编器
- `qemu-system-x86`：QEMU x86 模拟器（提供 `qemu-system-x86_64`）
- `gdb`：调试器

`make run` 会打开 QEMU 图形窗口；WSL2 需要启用 WSLg（Windows 11）或自行配置 X Server。

## 构建与运行

```bash
make          # 编译引导扇区与内核，生成 build/os-image.bin
make run      # 编译并启动 QEMU
make clean    # 清理 build/ 下的产物
```

- 默认目标为 `all`；`make run` 依赖 `all`，会先完成构建。
- 引导扇区一次从磁盘读取 128 个扇区（64 KiB），因此 `build/kernel.bin` 必须不超过 64 KiB；当前约 44 KiB。若内核继续增大，需要同时修改 `boot/boot.asm` 的读取扇区数和 `Makefile` 的镜像填充大小。
- 最终镜像为 512 B 引导扇区 + 64 KiB 内核区，并填充/截断到 66,048 B。
- 构建依赖 `find`、`stat`、`dd`、`truncate` 等 Linux 工具，请在 WSL/Linux 中执行，不要在 Windows `cmd` 中直接运行。
- `make clean` 只清理 `build/` 目录。

## 启动与内存布局

| 区域 | 地址 / 大小 | 说明 |
| --- | --- | --- |
| 引导扇区 | `0x7C00`，512 B | BIOS 加载并执行 |
| 内核 | `0x10000`，最多 64 KiB | 引导扇区用 `int 13h` 读入，链接脚本同样定位到 `0x10000` |
| 引导阶段栈 | `0x90000` | `boot.asm` 进入保护模式后设置 |
| VGA 文本缓冲 | `0xB8000` | 80×25，每字符 2 B |
| 可管理物理内存 | 16 MiB | 4096 个 4 KiB 页，位图记录占用 |
| 保留低端内存 | `0x00000`–`0xFFFFF` | 前 256 页不参与分配 |
| 内核堆上限 | 16 页 = 64 KiB | `MAX_HEAP_PAGES` |
| OS 镜像 | 66,048 B | 512 B 引导扇区 + 64 KiB 内核区 |

内核启动时使用链接脚本导出的 `__bss_start` / `__bss_end` 清空 `.bss`。当前内核镜像和 BSS 均位于 1 MiB 以下；堆页从 1 MiB 开始分配，分页建立 16 MiB 恒等映射后可直接访问。

## 调试与检查

```bash
# 查看内核入口地址（应为 0x10000）
readelf -h build/kernel.elf | grep Entry

# 反汇编内核开头，确认 _start
objdump -d build/kernel.elf | head -30

# 查看段布局与 BSS 范围
readelf -S build/kernel.elf

# 确认引导扇区末尾的 0xAA55 标志
od -An -tx1 -j 510 -N 2 build/boot.bin

# 查看最终镜像大小
stat -c '%n %s bytes' build/os-image.bin
```

使用 QEMU 的 gdbstub 调试：

```bash
# 终端 1：启动 QEMU，-s 开启 1234 端口 gdbstub，-S 暂停等待调试器
qemu-system-x86_64 -drive format=raw,file=build/os-image.bin -s -S

# 终端 2：加载内核符号并连接
gdb build/kernel.elf -ex "target remote :1234" -ex "break main" -ex "continue"
```

## 待实现功能（TODO）

以下功能尚未实现或只完成了框架。每完成一项，直接删除对应条目即可；全部完成后可删除本节。

- [ ] **用户态与 TSS**：GDT 目前只有 Ring 0 代码/数据段，所有任务都在内核态执行；尚未建立 TSS、Ring 3 段和用户栈。
- [ ] **独立进程地址空间**：分页只做 16 MiB 恒等映射，所有任务共享同一页目录；尚无按进程页目录、按需分页、写时复制和 swap。
- [ ] **CPU 异常处理**：IDT 尚未注册 0–31 号异常（#PF、#GP、#UD 等），出错时没有诊断信息。
- [ ] **磁盘与持久化文件系统**：无 ATA/IDE 驱动，ramfs 重启即丢失；尚未实现 FAT/EXT2 等持久化文件系统。
- [ ] **内存探测**：固定按 16 MiB 管理，未解析 BIOS E820 / Multiboot 内存映射，也无法利用更大内存。
- [ ] **系统调用完善**：目前只有 4 个调用，缺少文件、进程、内存相关接口和用户指针校验；打印类调用的参数约定也需要统一。
- [ ] **内核段权限**：当前恒等映射全部为 RW，链接器提示 LOAD 段 RWX；后续可把只读数据段单独映射为只读。
- [ ] **堆扩容与页回收**：堆最多 16 页（64 KiB），空闲页不会归还物理页分配器；可增加按需扩页和整页回收。
- [ ] **同步原语**：无互斥锁、信号量等机制，任务间共享数据没有保护。
- [ ] **驱动扩展**：仅 VGA 文本和 PS/2 键盘；无串口、RTC、鼠标、声卡、网卡等驱动。
- [ ] **Shell 增强**：管道、重定向、环境变量、行内编辑（左右键目前只移动光标）、`cp`/`mv`/递归删除等。
- [ ] **可执行程序加载**：无 ELF 加载器、initrd 和用户程序，任务只能由内核内函数创建。
- [ ] **调度增强**：固定 8 个任务槽、固定 2 KiB 栈、简单轮转，无优先级和时间片配置。
- [ ] **多核与高级中断**：无 SMP、ACPI、APIC/IOAPIC 支持，目前仅使用 8259 PIC。
- [ ] **自动化测试 / CI**：目前依赖人工在 QEMU 中验证。
