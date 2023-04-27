SECTIONS
{
    . = 0x80000;
   
    .text.boot : { *(.text.boot) }
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
   
    . = ALIGN(8);
   
    __bss_begin = .;
    .bss : { *(.bss* .bss.*) } 
    __bss_end = .;

    . = ALIGN(16);

    __heap_start = .;
    __kernel_end = .;

}
