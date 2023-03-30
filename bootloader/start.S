.section ".text.boot"

.global _start

_start:
    // only use core 0
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbnz    x1, hang

    // save x0~x3
    mov     x20, x0
    mov     x21, x1
    mov     x22, x2
    mov     x23, x3

    // set x28 = address of relocated main()
    ldr     x1, =main
    ldr     x2, =_start
    ldr     x3, =__bootloader_start
    sub     x1, x1, x2
    add     x1, x1, x3
    mov     x28, x1

clear_bss:
    ldr     x4, =__bss_start
    ldr     x5, =__bss_size
    cbz     x5, relocate
clear_bss_loop:
    str     xzr, [x4], #8
    sub     x5, x5, #1
    cbnz    x5, clear_bss_loop

relocate:
    mov     sp, x3
    ldr     x5, =__prog_size
    cbz     x5, run_main
relocate_loop:
    ldr     x4, [x2], #8
    str     x4, [x3], #8
    sub     x5, x5, #1
    cbnz    x5, relocate_loop

run_main:
    mov     x0, x20
    mov     x1, x21
    mov     x2, x22
    mov     x3, x23
    br      x28

hang:
    wfe
    b       hang