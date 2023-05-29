ENTRY(_start)
SECTIONS
{
	. = 0x80000;
	_loader_begin = .;
	.text :
	{
		KEEP(*(.text.boot))
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

	.data :
	{
		*(.data)
	}

	.bss ALIGN(16) (NOLOAD) :
	{
		__bss_start = .;
		*(.bss)
		__bss_end = .;
	}
	_loader_end = .;
	. = . + 1M;
}

__bss_size = (__bss_end - __bss_start) >> 3;
__stack_top = 0x3000000;
__relocate_point = 0x3000000;



