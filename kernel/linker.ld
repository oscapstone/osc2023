ENTRY(_start)

SECTIONS 
{
  . = 0x80000;
  _proc_start = .;
  _stext = .;
  .text : { 
    *(.text.boot)
    *(.text) 
    *(.rodata) 
  }
  . = ALIGN(0x1000);
  _etext = .;

  _sdata = .;
  .data : { *(.data) }
  . = ALIGN(0x1000);
  _edata = .;

  
  __bss_start = .;
  .bss (NOLOAD): { *(.bss*) } 
  . = ALIGN(0x1000);
  __bss_end = .;

  .heap (NOLOAD):
  {
    /*__heap_size = 16M;*/
    __heap_start = .;
    /*. += __heap_size;*/
    . += 0x1000000;
    __heap_end = .;
  }
  _proc_end = .;
}
