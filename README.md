# workspace-myos

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

