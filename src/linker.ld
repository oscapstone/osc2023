SECTIONS
{
  . = 0x80000;
  .text.boot : { *(.text.boot) }
  .text : { *(.text) }
  .=ALIGN(16);
  bss_begin = .;
  .bss : { *(.bss) }
  bss_end = .;
}
__bss_size = (bss_end - bss_begin)>>3;