_skernel = 0x80000;
/* The loading address of the initial ramdisk */
_max_ekernel = 0x8000000;

MEMORY
{
    RAM_KERNEL : ORIGIN = _skernel, LENGTH = _max_ekernel - _skernel
}

/* Must be 16-byte aligned.
   The highest address the ARM core can use. Total system SDRAM is 1G. Top 76M
   are reserved for VideoCore. */
_estack = 1024M - 76M;

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
