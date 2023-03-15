SECTIONS
{
    . = 0x80000;

    __prog_start = .;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
         *(.rodata .rodata.*)
    }

    .data : { 
        *(.data .data.*)
    }

    __prog_end = .;

    .bss : {
        . = ALIGN(16);

        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
}

__bss_size = (__bss_end - __bss_start) >> 3;
__prog_size = (__prog_end - __prog_start) >> 3;