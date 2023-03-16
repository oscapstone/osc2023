ENTRY(_start)
SECTIONS
{
    . = 0x60000;
    _loader_begin = .;
    .text.boot : { *(.text.boot) }
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    . = ALIGN(0x8);
    _bss_begin = .;
    .bss : { *(.bss*) }
    . = ALIGN(0x8);
    _bss_end = .;
    _loader_end = .;
    _stack_begin = . + 0x10000;
    _loader_size = _loader_end - _loader_begin;
    _second_kernel = 0x80000;
}