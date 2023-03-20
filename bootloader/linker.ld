ENTRY(_start)
SECTIONS
{
  . = 0x60000;
  _stext = .;
  .text : { 
    *(.text.relo)
    _bl_entry = .;
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

  
  _sbss = .;
  .bss (NOLOAD): { *(.bss*) } 
  . = ALIGN(0x1000);
  _ebss = .;

  .heap (NOLOAD):
  {
    /*__heap_size = 1M;*/
    __heap_start = .;
    /*. += __heap_size;*/
    . += 0x100000;
    __heap_end = .;
  }

  _blsize = _ebss - _stext;
}