/* Separete the text and data section in different
 * segment to avoid the warning of
 * "HAS a LOAD segment with RWX permissions"
 *
 * Reference: https://www.redhat.com/en/blog/linkers-warnings-about-executable-stacks-and-segments
 * Reference: https://web.archive.org/web/20120131063716/http://sources.redhat.com/binutils/docs-2.10/ld_3.html#SEC39
 */
PHDRS
{
	text PT_LOAD ;
	data PT_LOAD ;
}
SECTIONS
{
	. = 0x80000;
	.text : { KEEP( *(text.boot) ) *(.text .text.* .gnu.linkonce.t*) } :text
	/* Note: text.boot need to place at 0x80000 */
	/* KEEP: keep the section which will not be GC */
	.rodata : { *(.rodata .rodata.* .gnu.linkonce.d*) } :data
	.data : { *(.data .data.* .gnu.linkonce.d*) } :data
	.bss (NOLOAD) : { 
		. = ALIGN(16);
		__bss_start = .;
		*(.bss .bss.*) 
		*(COMMON) /* For multiple variable to link at same loc*/
		__bss_end = .;
	} :data
	_end = .;
	/DISCARD/ : {*(.comment) *(.gnu*) *(.note*) *(.eh_frame) } :data
}

__bss_size = (__bss_end - __bss_start) >> 3;


