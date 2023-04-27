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