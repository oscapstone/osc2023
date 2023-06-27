ENTRY(_start)
MEMORY
{
	CODE (rx) : ORIGIN = 0x80000, LENGTH = 128K
	ROM (r) : ORIGIN = 0xa0000, LENGTH = 128K
	RAM (rw) : ORIGIN = 0xc0000, LENGTH = 40M
	USER (rwx) : ORIGIN = 0x40c0000, LENGTH = 5M
}
SECTIONS
{
	.text :
	{ 
  		*(.text.start)
		*(.text)
	} >CODE
	.rodata :
	{
		_rodata_begin = .;
		*(.rodata)
		_rodata_end = .;
	} >ROM
	.data :
	{
		. = ALIGN(0x8);
		_data_begin = .;
		*(.data)
		. = ALIGN(0x8);
		_data_end = .;
	} >RAM
	.bss : {
		. = ALIGN(0x8);
		_bss_begin = .;
		*(.bss .bss.*)
		. = ALIGN(0x8);
		_bss_end = .;
	} >RAM
	_heap_begin = ORIGIN(RAM) + 20M;
	_heap_end = ORIGIN(RAM) + 35M;
	_stack_top = ORIGIN(RAM) + LENGTH(RAM);
	_user_begin = ORIGIN(USER);
	_user_end = ORIGIN(USER) + LENGTH(USER) - 8;
	_image_memory_begin = ORIGIN(CODE);
	_image_memory_end = ORIGIN(USER) + LENGTH(USER);
}
