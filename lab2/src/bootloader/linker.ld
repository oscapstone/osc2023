ENTRY(_start)
MEMORY
{
	BOOTLOADER (rx) : ORIGIN = 0x20000, LENGTH = 128K
	CODE (rx) : ORIGIN = 0x80000, LENGTH = 128K
	ROM (r) : ORIGIN = 0xa0000, LENGTH = 128K
	RAM (rw) : ORIGIN = 0xc0000, LENGTH = 512K
}
SECTIONS
{
	.text :
	{ 
		_loader_begin = .;
  		*(.text.start)
		*(.text.loader)
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
		_loader_end = .;
	} >RAM
	_stack_top = ORIGIN(RAM) + LENGTH(RAM);
	_bootloader_begin = ORIGIN(BOOTLOADER);
	_loader_size = _loader_end - _loader_begin;
	_kernel_begin = ORIGIN(CODE);
}