SECTIONS {
  . = 0x80000;
  .text : {
    KEEP(*(.text.boot))
    *(.text)
  }
  .rodata : {
    *(.rodata .rodata.*);
  }
  PROVIDE(_data = .);
  .data : {
    *(.data)
  }
  .bss : {
    . = ALIGN(16);
    __bss_start = .;
    *(.bss)
    __bss_end = .;
  }
  _end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3;
