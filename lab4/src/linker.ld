SECTIONS
{
  . = 0x80000;
  PROVIDE(_code = .);
  .text.boot : { *(.text.boot) }
  .text : { *(.text) }
  .rodata : { *(.rodata .gnu.linkonce.d*) }
  .data : { *(.data .gnu.linkonce.d*) }
  . = ALIGN(16);
  __bss_start = .;
  .bss : { *(.bss) }
  __bss_end = .;
  _end = .;
}
__bss_size = (__bss_end - __bss_start)>>3;
__loader_size = (_end - _code)>>3;