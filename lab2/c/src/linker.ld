_skernel = 0x80000;

MEMORY
{
    RAM_KERNEL : ORIGIN = _skernel,
                 /* Total system SDRAM is 1G. Top 76M are reserved for
                    VideoCore. */
                 LENGTH = (1024M - 76M) - _skernel
}

/* Must be 16-byte aligned. */
_estack = ORIGIN(RAM_KERNEL) + LENGTH(RAM_KERNEL);

ENTRY(_start)

SECTIONS
{
    .text :
    {
        _stext = .;

        *(.text._start) /* Entry point. See `start.S`. */
        *(.text)

        _etext = .;
    } > RAM_KERNEL

    .data :
    {
        _sdata = .;

        *(.data)

        _edata = .;
    } > RAM_KERNEL

    /* The .bss section is 16-byte aligned to allow the section to be
       zero-initialized with the `stp` instruction without using unaligned
       memory accesses. See `start.S`. */
    .bss : ALIGN(16)
    {
        _sbss = .;

        *(.bss)
        *(COMMON)

        . = ALIGN(16);
        _ebss = .;
    } > RAM_KERNEL
}
