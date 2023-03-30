.section ".text.boot"

.global _start

_start:
    // init .bss
    LDR     x1, =__sbss
    LDR     x2, =__bss_size
    B       2f
1:  STR     xzr, [x1], #8
    SUB     x2, x2, #1
2:  CBNZ    x2, 1b

    // set stack pointer
    LDR     x1, =_start
    MOV     sp, x1

    // start kernel, this call should not return
    BL      start_kernel

3:  WFE
    B       3b