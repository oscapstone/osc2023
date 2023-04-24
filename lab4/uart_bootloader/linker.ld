SECTIONS
{
    . = 0x80000;

    __begin = .;

    .text.boot : { *(.text.boot) }
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }

    . = ALIGN(8);

    __bss_begin = .;
    .bss : { *(.bss* .bss.*) } 
    __bss_end = .;

    __end = .;

}

__relocation = 0x60000;