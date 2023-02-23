SECTIONS
{
	. = 0x80000;
	.text : { KEEP( *(text.boot) ) *(.text .text.* .gnu.linkonce.t*) }
	/* Note: text.boot need to place at 0x80000 */
	/* KEEP: keep the section which will not be GC */
	.rodata : { *(.rodata .rodata.* .gnu.linkonce.d*) }
	.data : { *(.data .data.* .gnu.linkonce.d*) }
	.bss (NOLOAD) : { 
		. = ALIGN(16);
		__bss_start = .;
		*(.bss .bss.*) 
		*(COMMON) /* For multiple variable to link at same loc*/
		__bss_end = .;
	}
	_end = .;
	/DISCARD/ : {*(.comment) *(.gnu*) *(.note*) *(.eh_frame)}
}

__bss_size = (__bss_end - __bss_start) >> 3;


