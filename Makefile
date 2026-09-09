NASM = nasm
GCC = gcc
LD = ld
OBJCOPY = objcopy

IMAGE = os-image.bin
BOOT_BIN = build/boot.bin
KERNEL_ELF = build/kernel.elf
KERNEL_BIN = build/kernel.bin

# 所有 C 源文件对应的目标文件
KERNEL_C_SOURCES = $(wildcard kernel/*.c)
KERNEL_C_OBJECTS = $(patsubst kernel/%.c, build/%.o, $(KERNEL_C_SOURCES))

# 所有汇编源文件对应的目标文件
KERNEL_ASM_OBJECTS = build/interrupt.o build/switch.o

# 所有内核目标文件（用于依赖）
KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

all: $(IMAGE)

# 引导扇区
$(BOOT_BIN): boot/boot.asm
	$(NASM) -f bin boot/boot.asm -o $(BOOT_BIN)

# 汇编文件编译
build/interrupt.o: kernel/interrupt.asm
	$(NASM) -f elf32 kernel/interrupt.asm -o build/interrupt.o

build/switch.o: kernel/switch.asm
	$(NASM) -f elf32 kernel/switch.asm -o build/switch.o

# C 文件编译
build/%.o: kernel/%.c
	$(GCC) -m32 -ffreestanding -fno-pic -fno-pie -c $< -o $@

# 链接内核：必须将 kernel.o（包含 main）放在第一位
$(KERNEL_ELF): $(KERNEL_OBJECTS) kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o $(KERNEL_ELF) \
		build/kernel.o build/idt.o build/screen.o build/scheduler.o \
		build/interrupt.o build/switch.o

# 生成纯二进制内核
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)

# 生成磁盘映像：引导扇区 + 内核（填充至 16 个扇区）
$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(IMAGE)
	dd if=/dev/zero bs=1 count=$$((8192 - $$(stat -c%s $(KERNEL_BIN)))) >> $(IMAGE) 2>/dev/null
	truncate -s 8704 $(IMAGE)

run: all
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

clean:
	rm -rf build/* $(IMAGE)