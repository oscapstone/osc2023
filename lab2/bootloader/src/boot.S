.section ".text.boot"

.global _start

_start:
// relocate bootloader text section to place before 0x80000 before execute
    mov     x1, 0x80000
    mov     x2, 0x60000
    ldr     w3, =__loader_size
1:  ldr     x4, [x1], #8
    str     x4, [x2], #8
    sub     w3, w3, #1
    cbnz    w3, 1b
// if counter w3 not equal to 0, then loop (jump to tag "1" "backward" till finish)
    ldr     x1, 0x60000
    mov     sp, x1

// should clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
// which xzr is the arm zero register
    sub     w2, w2, #1
    cbnz    w2, 3b
// if counter w2 not equal to 0, then loop (jump to tag "3" "backward" till finish)
4:  bl      5f
    mov     sp, 0x60000
    mov     x6, 0x80000
    str     xzr, [x6]
    bl      kernel_main
5:  sub     x30, x30, #(0x80000-0x60000)
    ret