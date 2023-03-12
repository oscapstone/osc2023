_stack_top = 0x400000;

SECTIONS
{
  . = 0x0;
  
  bss_begin = .;
  .bss :{
    *(.bss*)
  }
  bss_end = .;  
  . = 0x80000;

  .text.boot :{
    *(text.boot)
  }

  .text : { 
    *(.text) 
  }

  .rodata : {
    *(.rodata)
  }

  .data : {
    *(.data)
  }

  . = _stack_top;
  _sstack = .;
}