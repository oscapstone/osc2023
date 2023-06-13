ENTRY(_start)
MEMORY
{
	CODE (rx) : ORIGIN = 0x80000, LENGTH = 128K
	ROM (r) : ORIGIN = 0xa0000, LENGTH = 128K
	RAM (rw) : ORIGIN = 0xc0000, LENGTH = 5M
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
		_data_begin = .;
		*(.data)
		_data_end = .;
	} >RAM
	.bss : {
		. = ALIGN(0x8);
		_bss_begin = .;
		*(.bss)
		_bss_end = .;
	} >RAM
	_heap_begin = ORIGIN(RAM) + 1M;
	_heap_end = ORIGIN(RAM) + 4M;
	_stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
