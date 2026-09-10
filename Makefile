NASM = nasm
GCC = gcc
LD = ld
OBJCOPY = objcopy

IMAGE = build/os-image.bin
BOOT_BIN = build/boot.bin
KERNEL_ELF = build/kernel.elf
KERNEL_BIN = build/kernel.bin

# 内核 C 源文件（自动查找）
KERNEL_C_SOURCES = $(shell find kernel -name '*.c')
KERNEL_C_OBJECTS = $(patsubst kernel/%.c, build/%.o, $(KERNEL_C_SOURCES))

# 内核汇编源文件
KERNEL_ASM_OBJECTS = build/arch/start.o build/arch/switch.o build/arch/interrupt.o

# 所有内核对象
KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

all: $(IMAGE)

# 引导扇区
$(BOOT_BIN): boot/boot.asm
	@mkdir -p build
	$(NASM) -f bin boot/boot.asm -o $(BOOT_BIN)

# 汇编
build/arch/%.o: kernel/arch/%.asm
	@mkdir -p build/arch
	$(NASM) -f elf32 $< -o $@

# C 文件（保持子目录结构）
build/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(GCC) -m32 -ffreestanding -fno-pic -fno-pie -c $< -o $@

# 链接（kernel.c 在第一位确保 main 入口）
$(KERNEL_ELF): $(KERNEL_OBJECTS) kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o $(KERNEL_ELF) \
		build/arch/start.o \
		build/kernel.o \
		build/drivers/screen.o \
		build/interrupt/idt.o \
		build/mm/memory.o \
		build/mm/heap.o \
		build/sched/scheduler.o \
		build/arch/interrupt.o \
		build/arch/switch.o

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)

# 磁盘映像：引导扇区 + 内核（填充至 32KB）
$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	@mkdir -p build
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(IMAGE)
	dd if=/dev/zero bs=1 count=$$((32768 - $$(stat -c%s $(KERNEL_BIN)))) >> $(IMAGE) 2>/dev/null
	truncate -s 33280 $(IMAGE)

run: all
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

clean:
	rm -rf build/*

.PHONY: all run clean