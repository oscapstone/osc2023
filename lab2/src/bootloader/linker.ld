bootloader_start	= 0x60000;
stack_top			= 0x400000;
start_address		= 0x6000000; /* available mem */
end_address			= 0x7ffffff;
SECTIONS
{
	. = bootloader_start;
	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.got) *(.got.plt) *(.data) }
	. = ALIGN(0x8);
	data_end = .;
	bss_begin = .;
	.bss : { *(.bss*) } 
	bss_end = .;

	. = ALIGN(0x8);
	heap_start = .;
	heap_end = stack_top;
	. = stack_top;
}
