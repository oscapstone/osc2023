_suser = 0;

ENTRY(_start)

SECTIONS
{
    . = _suser;

    .text :
    {
        _stext = .;

        *(.text._start) /* Entry point. See `start.S`. */
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(.text .text.*)
        *(.eh_frame)
        *(.eh_frame_hdr)

        _etext = .;
    }

    .rodata :
    {
        _srodata = .;

        *(.rodata .rodata.*)

        _erodata = .;
    }

    .data :
    {
        _sdata = .;

        *(.data .data.*)

        _edata = .;
    }

    .bss :
    {
        _sbss = .;

        *(.bss .bss.*)
        *(COMMON)

        _ebss = .;
    }

    _euser = .;
}
