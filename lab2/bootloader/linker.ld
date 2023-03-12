SECTIONS
{
  . = 0x80000;
  .text.boot : { *(.text.boot) }
  .text : { *(.text .text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data) }
  . = ALIGN(0x8);
  __bss_begin = .;
  .bss : { *(.bss .bss*) }
  __bss_end = .;
}
