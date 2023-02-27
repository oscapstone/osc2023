SECTIONS
{
    . = 0x80000;
    /* Puts the OS startup code inside this section */
    .text.boot : { *(.text.boot) }
    .text :  { *(.text) }
    /* Read-only data */
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    /* Aligns in order to use `str` instrustion */
    . = ALIGN(0x8);
    /* .bss should be initialized to 0 */
    /* Records start and end of this section */
    bss_begin = .;
    .bss : { *(.bss*) } 
    bss_end = .;
}