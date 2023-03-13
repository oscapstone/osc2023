.section ".text"

.global init

init:
    ldr     x1, =_start
    mov     sp, x1

    ldr     x1, =__bss_start
    ldr     w2, =__bss_size

// clear bss
1:
    cbz     w2, 2f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 1b

2:
    bl      main

3:
    b       3b
