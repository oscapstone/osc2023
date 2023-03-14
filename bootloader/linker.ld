__bootloader = 0x60000;
__kernel = 0x80000;
SECTIONS
{
  . = __bootloader;
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