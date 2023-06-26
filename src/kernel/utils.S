.globl write_reg_32
write_reg_32:
    str w1, [x0]
    ret

.globl read_reg_32
read_reg_32:
    ldr w0, [x0]
    ret

.globl delay
delay:
    subs x0, x0, #1
    bne delay
    ret 

.global delay_test
delay_test:
    subs x0, x0, #1
    mov x1, 0x65
    bne delay
    // bl uart_send
    ret 

.global gen_trapframe
gen_trapframe:
    mov x0, 272
    mov x1, lr;
    bl kmalloc
    stp x0, x1, [x0 ,16 * 0]
    stp x2, x3, [x0 ,16 * 1]
    stp x4, x5, [x0 ,16 * 2]
    stp x6, x7, [x0 ,16 * 3]
    stp x8, x9, [x0 ,16 * 4]
    stp x10, x11, [x0 ,16 * 5]
    stp x12, x13, [x0 ,16 * 6]
    stp x14, x15, [x0 ,16 * 7]
    stp x16, x17, [x0 ,16 * 8]
    stp x18, x19, [x0 ,16 * 9]
    stp x20, x21, [x0 ,16 * 10]
    stp x22, x23, [x0 ,16 * 11]
    stp x24, x25, [x0 ,16 * 12]
    stp x26, x27, [x0 ,16 * 13]
    stp x28, x29, [x0 ,16 * 14]
    stp x30, x0, [x0, 16 * 15]
    mov x2, sp
    stp x1, x2, [x0, 16 * 16]
    mov lr, x1
    ret