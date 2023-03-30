ENTRY(start)
SECTIONS
{
    . = 0x100000;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    _bss_begin = .;
    .bss : { *(.bss*) }
    . = ALIGN(0x8);
    _bss_end = .;
    _heap_begin = . + 0x5000;
    _stack_begin = . + 0x10000;
}