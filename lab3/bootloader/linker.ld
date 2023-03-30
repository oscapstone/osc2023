SECTIONS
{
    . = 0x80000;

    __begin = .;

    .text : { KEEP(*(.text.boot)) *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    .data : { *(.data) }
    .bss ALIGN(0x8) : {
        __bss_start = .; 
        *(.bss)
        __bss_end = .;
    }

    . = ALIGN(0x8);
    __end = .;
}

__relocation = 0x60000;
