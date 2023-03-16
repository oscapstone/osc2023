SECTIONS
{
    . = 0x80000;
    __start = .;
    .text : { KEEP(*(.text.boot)) *(.text .text.*) }
    .bss : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss.*)
        __bss_end = .;
    }
    __end = .;

}
__bss_size = (__bss_end - __bss_start) >> 3;
