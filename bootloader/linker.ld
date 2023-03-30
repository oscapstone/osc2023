__bootloader = 0x60000;
__kernel = 0x80000;
SECTIONS
{
  . = __kernel;
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