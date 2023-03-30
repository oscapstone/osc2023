_bootloader = 0x60000;
_kernel     = 0x80000;
_stack_top  = 0x400000;

SECTIONS
{
  . = _bootloader;

  .text.boot : {
    *(.text.boot)
  }

  .text : {
    *(.text)
  }

  .rodata : {
    *(.rodata)
  }

  .data : {
    *(.got)
    *(.got.plt)
    *(.data)
  }

  end_data = .;
  
  bss_begin = .;
  .bss : {
    *(.bss*)
  }
  bss_end = .;

  . = _stack_top;
  _sstack = .;
}
