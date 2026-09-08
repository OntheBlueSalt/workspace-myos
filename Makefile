NASM = nasm
GCC = gcc
LD = ld
OBJCOPY = objcopy

IMAGE = os-image.bin
BOOT_BIN = build/boot.bin
KERNEL_ELF = build/kernel.elf
KERNEL_BIN = build/kernel.bin

# 收集所有内核源文件
KERNEL_C_SOURCES = $(wildcard kernel/*.c)
KERNEL_OBJECTS = $(patsubst kernel/%.c, build/%.o, $(KERNEL_C_SOURCES))

all: $(IMAGE)

$(BOOT_BIN): boot/boot.asm
	$(NASM) -f bin boot/boot.asm -o $(BOOT_BIN)

# 编译所有内核 .c 文件为 .o
build/%.o: kernel/%.c
	$(GCC) -m32 -ffreestanding -fno-pic -fno-pie -c $< -o $@

# 链接所有内核 .o 文件为 ELF
$(KERNEL_ELF): $(KERNEL_OBJECTS) kernel/linker.ld
	$(LD) -m elf_i386 -T kernel/linker.ld -o $(KERNEL_ELF) $(KERNEL_OBJECTS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)

$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(IMAGE)
	dd if=/dev/zero bs=1 count=$$((4096 - $$(stat -c%s $(KERNEL_BIN)))) >> $(IMAGE) 2>/dev/null
	truncate -s 4608 $(IMAGE)

run: all
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

clean:
	rm -rf build/* $(IMAGE)