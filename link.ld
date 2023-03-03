SECTIONS
{
    . = 0x80000;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
         *(.rodata .rodata.*)
    }

    .data : { 
        *(.data .data.*)
    }

    .bss : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
}

__bss_size = (__bss_end - __bss_start)>>3;
