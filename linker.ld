SECTIONS
{
  . = 0x80000;
  .text : {
    *(.text.boot)
    *(.text)
  }
  .rodata : { *(.rodata.*) }
  .bss : {
    __sbss = .;
    *(.bss)
    __ebss = .;
  }
}
__bss_size = (__ebss - __sbss) >> 3;