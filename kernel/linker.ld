_proc_start = 0xffff000000080000;


ENTRY(_start)
SECTIONS 
{
  . = _proc_start;
  
  _stext = .;
  .text : { 
    *(.text.boot)
    *(.text) 
    *(.rodata) 
  }
  . = ALIGN(0x1000);
  _etext = .;

  _sdata = .;
  .data (NOLOAD): { *(.data) }
  . = ALIGN(0x1000);
  _edata = .;

  
  __bss_start = .;
  .bss (NOLOAD): { *(.bss*) } 
  . = ALIGN(0x1000);
  __bss_end = .;

  .heap (NOLOAD):
  {
    __heap_start = .;
    . += 0x1000000;
    __heap_end = .;
  }
  _proc_end = .;
  
}