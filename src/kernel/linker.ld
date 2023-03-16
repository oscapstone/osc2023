ENTRY(_start)
SECTIONS
{
    . = 0x80000;
    .text.boot : { *(.text.boot) }
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    _bss_begin = .;
    .bss : { *(.bss*) }
    . = ALIGN(0x8);
    _bss_end = .;
    _stack_begin = . + 0x10000;
    _heap_begin = . + 0x12000;
}