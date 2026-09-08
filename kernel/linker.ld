ENTRY(main)
SECTIONS
{
    . = 0x10000;       /* 内核加载地址 */
    .text : { *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}