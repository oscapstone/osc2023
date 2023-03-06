_heap_stack_size = 1M;
_new_kernel_size = 256K;

SECTIONS
{
    /* bootloader 0x60000(config.txt), kernel 0x80000 */
    . = 0x80000;
    _start = .;
    .text : { *(.text.boot) *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : {
        _bss_top = .;
        *(.bss) 
    }
    _bss_size = SIZEOF(.bss) >> 3;
    .heap : {
        . = ALIGN(8);
        _heap_top = .;
    } 
    . = . + _heap_stack_size;
    .stack : {
        . = ALIGN(8);
        _stack_top = .;
    }
    . = 0x3000000;
    _kernel_start_addr = 0x3000000;
    . = . + _new_kernel_size;
    _end = .;

}
