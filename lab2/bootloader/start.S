.section ".text.boot"

.globl _start

_start:
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, _one

_loop:
    wfe
    b       _loop

_one:
    ldr     x0, =_start
    mov     sp, x0

    adr     x0, __bss_begin
    adr     x1, __bss_end
    sub    x1, x1, x0
_clean_bss:
    cbz    x1, _bootloader
    str     xzr, [x0], #8
    subs    x1, x1, #8
    cbnz    x1, _clean_bss
_bootloader:
    bl  bootloader_main
    b   _loop
