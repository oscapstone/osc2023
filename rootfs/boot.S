.section ".text.boot"
.globl _start
_start:

    mov x10, x0
    mov x11, x1
    mov x12, x2
    mov x13, x3

master:
    ldr x1, =0x90000
    mov sp, x1

    adr x0, _bss_begin
    adr x1, _bss_end
    sub x1, x1, x0
    bl memzero

    mov x0, 0x80000
    mov x1, 0x60000
    ldr x2, =_loader_size
    bl _memcpy_64

    ldr x1, =_stack_begin
    mov sp, x1
    b #(main-0x20000)

.globl memzero
memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt memzero
    ret

_memcpy_64:
    ldr x4, [x0], #8
    str x4, [x1], #8
    subs x2, x2, #8
    b.gt _memcpy_64
    ret