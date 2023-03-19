SECTIONS
{
  . = 0x80000;
  .text : {
    *(.text.boot)
    *(.text)
  }
  .rodata : { *(.rodata.*) }
  .data : { *(.data) }
  .bss : {
    __sbss = .;
    *(.bss)
    __ebss = .;
  }
}
__bss_size = (__ebss - __sbss) >> 3;