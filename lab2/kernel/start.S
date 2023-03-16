.section ".data"

.globl DTB_ADDR

.align 4
DTB_ADDR:
    .quad 0

.section ".text.boot"

.globl _start

_start:
    ;ldr     x2, =0x5FFF8
    ;ldr     x3, [x2]
    mov     x3, x0
    ldr     x1, =DTB_ADDR
    str     x3, [x1]
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
    cbz    x1, _kernel
    str     xzr, [x0], #8
    subs    x1, x1, #8
    cbnz    x1, _clean_bss
_kernel:
    bl  kernel_main
    b   _loop
