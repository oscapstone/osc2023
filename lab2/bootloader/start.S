.section ".text.boot"

.globl _start

_start:
    ldr     x1, =0x5FFF8
    str     x0, [x1]
    mov     x28, x0
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
    sub     x1, x1, x0
_clean_bss:
    cbz     x1, _relocate_start
    str     xzr, [x0], #8
    subs    x1, x1, #8
    cbnz    x1, _clean_bss

_relocate_start:
    ldr     x0, =__begin
    ldr     x1, =__end
    ldr     x2, =_relocate_addr
    sub     x1, x1, x0

_relocate_body:
    ldr     x4, [x0], #8
    str     x4, [x2], #8
    sub     x1, x1, #8
    cbnz    x1, _relocate_body

_bootloader:
    ldr     x0, =bootloader_main
    ldr     x1, =__begin
    ldr     x2, =_relocate_addr
    sub     x0, x0, x1
    add     x0, x0, x2
    mov     sp, #0x80000
    br      x0
    ; bl  bootloader_main
    b   _loop
