_stack_top = 0x400000;
start_mem  = 0x6000000;
end_mem    = 0x7ffffff;
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