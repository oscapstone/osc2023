SECTIONS
{
    . = 0x70000;
   _code_relocate_place = .;
    /*bootloader on 0x80000, kernel on 0x80000 need to self relocate bootloader to other place*/
    . = 0x80000;
    PROVIDE(_start = .);
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;
}
__bss_size = (__bss_end - __bss_start)>>3;
__code_size = (_end - _start);
