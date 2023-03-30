__heap_stack_size = 1M;

SECTIONS
{
    . = 0x80000; /* 512KB */
    _start = .;
    .text : { *(.text.boot) *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data ) }
    .bss : {
        __bss_start = .;
        *(.bss)
        
        __bss_end = .;
    }    
    __bss_size = (__bss_end - __bss_start)>>3;
    .heap : {
        . = ALIGN(8);
        __heap_start = .;
    }     
    . = . + __heap_stack_size;
    .stack : {
        . = ALIGN(8);
        __stack_start = .;
    }
    _end = .;
}
