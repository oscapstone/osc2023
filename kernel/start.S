.section ".text.boot"

.global _start

_start:
    ldr     x28, =_start
    mov     sp, x28

    ldr     x4, =__bss_start
    ldr     x5, =__bss_size
    cbz     x5, run_main

clear_bss_loop:
    str     xzr, [x4], #8
    sub     x5, x5, #1
    cbnz    x5, clear_bss_loop

run_main:
    b      main

hang:
    b       hang
