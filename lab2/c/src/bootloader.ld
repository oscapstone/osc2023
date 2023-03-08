_sbootloader = 0x60000;
_skernel = 0x80000;
/* See linker.ld. */
_max_ekernel = 1024M - 76M;

MEMORY
{
    RAM_BOOTLOADER : ORIGIN = _sbootloader, LENGTH = _skernel - _sbootloader
}

/* Must be 16-byte aligned. */
_estack = ORIGIN(RAM_BOOTLOADER) + LENGTH(RAM_BOOTLOADER);

ENTRY(_start)

SECTIONS
{
    .text :
    {
        _stext = .;

        *(.text._start) /* Entry point. See `start.S`. */
        *(.text)

        _etext = .;
    } > RAM_BOOTLOADER

    .data :
    {
        _sdata = .;

        *(.data)

        _edata = .;
    } > RAM_BOOTLOADER

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
    } > RAM_BOOTLOADER
}
