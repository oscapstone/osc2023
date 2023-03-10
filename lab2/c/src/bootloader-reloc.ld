_sbootloader = 0x60000;
_skernel = 0x80000;
/* See linker.ld. */
_max_ekernel = 0x8000000;

MEMORY
{
    RAM_BOOTLOADER : ORIGIN = _sbootloader, LENGTH = _skernel - _sbootloader
    RAM_BOOTLOADER_LOAD : ORIGIN = _skernel, LENGTH = _skernel - _sbootloader
}

/* Must be 16-byte aligned. */
_estack = ORIGIN(RAM_BOOTLOADER) + LENGTH(RAM_BOOTLOADER);

ENTRY(_reloc)

SECTIONS
{
    /* Contains the code for relocation. Not itself relocated. */
    .text._reloc :
    {
        *(.text._reloc)
    } > RAM_BOOTLOADER_LOAD

    /* The following sections are 16-byte aligned to allow the sections to be
       relocated or zero-initialized with the `stp` instruction without using
       unaligned memory accesses. See `bootloader-reloc.S` and `start.S`. */

    .text : ALIGN(16)
    {
        _stext = .;

        *(.text)
        *(.text*)
        *(.eh_frame)

        . = ALIGN(16);
        _etext = .;
    } > RAM_BOOTLOADER AT> RAM_BOOTLOADER_LOAD

    .rodata : ALIGN(16)
    {
        _srodata = .;

        *(.rodata)

        . = ALIGN(16);
        _erodata = .;
    } > RAM_BOOTLOADER AT> RAM_BOOTLOADER_LOAD

    .data : ALIGN(16)
    {
        _sdata = .;

        *(.data)

        . = ALIGN(16);
        _edata = .;
    } > RAM_BOOTLOADER AT> RAM_BOOTLOADER_LOAD

    .bss : ALIGN(16)
    {
        _sbss = .;

        *(.bss)
        *(COMMON)

        . = ALIGN(16);
        _ebss = .;
    } > RAM_BOOTLOADER AT> RAM_BOOTLOADER_LOAD
}

_sitext   = LOADADDR(.text);
_sirodata = LOADADDR(.rodata);
_sidata   = LOADADDR(.data);
