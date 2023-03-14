.section ".text.boot"

.global _start

_start:
    // freeze cpu if cpu id != 0
    MRS     x1, mpidr_el1
    AND     x1, x1, #3
    CBNZ    x1, 3f

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

    // load kernel
    BL      load_kernel
    // jump to kernel, this call should not return
    BL      __kernel

3:  WFE
    B       3b
