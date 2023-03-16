_heap_stack_size = 1M;

ENTRY(_start)
SECTIONS
{
	. = 0x80000;
	
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

	.heap : 
	{
		. = ALIGN(16);
		_heap_top = .;
	}


    . = ALIGN(32);
    _stack_bottom = .;
    . = . + _heap_stack_size;
    _stack_top = .;
    _end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3;

