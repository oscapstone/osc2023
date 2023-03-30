__bootloader_start = 0x60000;
__default_start = 0x80000;

__heap_size = 0x1000000;

SECTIONS
{
    . = __default_start;

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

    .heap : {
        . = ALIGN(16);
        __heap_start = .;
    }
    . = . + __heap_size;
    __heap_end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3;
__prog_size = (__bss_end - __prog_start) >> 3;