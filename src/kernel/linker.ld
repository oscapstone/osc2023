ENTRY(_start)
SECTIONS
{
    . = 0xffff000000000000;
    . += 0x80000;
    .text.boot : { *(.text.boot*) }
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    . = ALIGN(0x10);
    _bss_begin = .;
    .bss : { *(.bss*) }
    . = ALIGN(0x10);
    _bss_end = .;
    _stack_begin = 0xffff000000380000;
    _heap_begin = 0xffff000000380000;
    _heap_end = 0xffff000001010000;
}