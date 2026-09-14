ENTRY(_start)
SECTIONS
{
    . = 0x10000;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }
    .bss_end_marker : {
        BYTE(0);
    }
}