SECTIONS
{
  . = 0x80000;
  .text : { KEEP(*(.text.boot)) *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }
  . = ALIGN(0x8);
  __bss_begin = .;
  .bss : { *(.bss .bss*) }
  __bss_end = .;
}
