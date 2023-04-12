/*
Memory map: (End addresses are exclusive)
0x       0 (  0B) - 0x   80000 (512K): Reserved for firmware
0x   80000 (512K) - 0x 4000000 ( 64M): Kernel text, rodata, data, bss, heap
0x 4000000 ( 64M) - 0x 8000000 (128M): User program
0x 8000000 (128M) - 0x3a400000 (932M): Reserved for initramfs
0x3a400000 (932M) - 0x3ac00000 (940M): User program stack
0x3ac00000 (940M) - 0x3b400000 (948M): Kernel stack
*/

_skernel = 0x80000;
_max_ekernel = 0x4000000;

MEMORY
{
    RAM_KERNEL : ORIGIN = _skernel, LENGTH = _max_ekernel - _skernel
}

/* Must be 16-byte aligned.
   The highest address the ARM core can use. Total system SDRAM is 1G. Top 76M
   are reserved for VideoCore. */
_estack = 1024M - 76M;
/* Again, must be 16-byte aligned. */
_euserstack = _estack - 8M;

_suser = 64M;
_max_euser = 128M;

ENTRY(_start)

SECTIONS
{
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
    } >RAM_KERNEL

    .rodata :
    {
        _srodata = .;

        *(.rodata .rodata.*)

        _erodata = .;
    } >RAM_KERNEL

    .data :
    {
        _sdata = .;

        *(.data .data.*)

        _edata = .;
    } >RAM_KERNEL

    /* The .bss section is 16-byte aligned to allow the section to be
       zero-initialized with the `stp` instruction without using unaligned
       memory accesses. See `start.S`. */
    .bss : ALIGN(16)
    {
        _sbss = .;

        *(.bss .bss.*)
        *(COMMON)

        . = ALIGN(16);
        _ebss = .;
    } >RAM_KERNEL

    _ekernel = .;
}
