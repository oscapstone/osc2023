ENTRY(_start)
MEMORY
{
	CODE (rx) : ORIGIN = 0x80000, LENGTH = 512K
	ROM (r) : ORIGIN = 0x100000, LENGTH = 512K
	RAM (rw) : ORIGIN = 0x180000, LENGTH = 1M
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
		_bss_begin = .;
		*(.bss)
		_bss_end = .;
	} >RAM
	_stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
