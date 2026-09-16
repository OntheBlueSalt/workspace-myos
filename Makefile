NASM    = nasm
GCC     = gcc
LD      = ld
OBJCOPY = objcopy

IMAGE      = build/os-image.bin
BOOT_BIN   = build/boot.bin
KERNEL_ELF = build/kernel.elf
KERNEL_BIN = build/kernel.bin

# 自动查找所有 C 源文件
KERNEL_C_SOURCES = $(shell find kernel -name '*.c')
KERNEL_C_OBJECTS = $(patsubst kernel/%.c, build/%.o, $(KERNEL_C_SOURCES))

# 汇编源文件
KERNEL_ASM_OBJECTS = build/arch/start.o build/arch/interrupt.o build/arch/switch.o

# 所有内核对象（用于依赖）
KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

# 链接顺序（start.o 必须第一，其余按依赖顺序排）
LINK_OBJECTS = \
	build/arch/start.o \
	build/kernel.o \
	build/lib/string.o \
	build/fs/ramfs.o \
	build/shell/shell.o \
	build/drivers/screen.o \
	build/interrupt/idt.o \
	build/mm/memory.o \
	build/mm/heap.o \
	build/sched/scheduler.o \
	build/sched/tasks.o \
	build/arch/interrupt.o \
	build/arch/switch.o

all: $(IMAGE)

# 引导扇区
$(BOOT_BIN): boot/boot.asm
	@mkdir -p build
	$(NASM) -f bin boot/boot.asm -o $(BOOT_BIN)

# start.asm 单独规则
build/arch/start.o: kernel/arch/start.asm
	@mkdir -p build/arch
	$(NASM) -f elf32 $< -o $@

# 其他汇编
build/arch/%.o: kernel/arch/%.asm
	@mkdir -p build/arch
	$(NASM) -f elf32 $< -o $@

# C 文件（保留子目录结构）
build/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(GCC) -m32 -ffreestanding -fno-pic -fno-pie -Ikernel/lib -c $< -o $@

# 链接内核
$(KERNEL_ELF): $(KERNEL_OBJECTS) kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o $(KERNEL_ELF) $(LINK_OBJECTS)

# 内核 ELF -> 纯二进制
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)

# 拼接引导扇区 + 内核，填充到 32KB
$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(IMAGE)
	dd if=/dev/zero bs=1 count=$$((65536 - $$(stat -c%s $(KERNEL_BIN)))) >> $(IMAGE) 2>/dev/null
	truncate -s 66048 $(IMAGE)

run: all
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

clean:
	rm -rf build/*

.PHONY: all run clean