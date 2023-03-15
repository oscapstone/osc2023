.section ".text.boot"

.global _start

_start:
    ldr     x1, =_start
    mov     sp, x1

    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
    cbz     w2, run_main

clear_bss:
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, clear_bss

run_main:
    bl      main

hang:
    b       hang
