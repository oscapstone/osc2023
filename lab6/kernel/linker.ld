SECTIONS
{
    . = 0xffff000000000000;
    . += 0x80000;
   
    .text.boot : { *(.text.boot) }
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
   
    . = ALIGN(8);
   
    __bss_begin = .;
    .bss : { *(.bss* .bss.*) } 
    __bss_end = .;

    . = ALIGN(0x00001000);
    pg_dir = .;
	.data.pgd : { . += (3 * (1 << 12)); }

    __heap_start = .;
    __kernel_end = .;

}
