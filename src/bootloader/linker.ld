_bootloader = 0x60000;
_kernel     = 0x80000;
_stack_top  = 0x400000;

/* Define these just to prevent the linker from complaining */
_etext_user_shared = 0;
_stext_user_shared = 0;
_early_mem_base  = 0x7000000;
_early_mem_end  = 0x7ffffff;

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
