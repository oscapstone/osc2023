kernel_start		= 0x80000;
stack_top			= 0x400000;

SECTIONS
{
	. = kernel_start;
	.text.boot : { KEEP(*(.text.boot)) }
	.text : { *(.text .text.*) }
	.rodata : { *(.rodata) }
	.data : { *(.got) *(.got.plt) *(.data) }
	. = ALIGN(0x8);
	bss_begin = .;
	.bss : { *(.bss*) } 
	bss_end = .;
	
	. = ALIGN(0x8);
	heap_start = .;
	heap_end = stack_top;
	. = stack_top;
}
