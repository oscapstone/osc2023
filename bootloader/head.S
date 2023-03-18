.section ".text.boot"

.global _start

_start:
    // freeze cpu if cpu id != 0
    MRS     x1, mpidr_el1
    AND     x1, x1, #3
    CBNZ    x1, 3f

    // bootloader self-relocation
    ADR     x1, _start
    ADR     x2, __sbss
    SUB     x2, x2, x1
    ADD     x2, x2, #7
    LSR     x2, x2, #3
    LDR     x3, =__bootloader
    B       5f
4:  LDR     x4, [x1], #8
    STR     x4, [x3], #8
    SUB     x2, x2, #1
5:  CBNZ    x2, 4b

    // jump to new _after_relocate
    LDR     x1, =_start
    LDR     x3, =_after_relocate
    SUB     x3, x3, x1
    LDR     x1, =__bootloader
    ADD     x1, x1, x3
    BR      x1

_after_relocate:
    // init .bss
    LDR     x1, =__sbss
    LDR     x2, =__ebss
    SUB     x2, x2, x1
    ADD     x2, x2, #7
    LSR     x2, x2, #3
    B       2f
1:  STR     xzr, [x1], #8
    SUB     x2, x2, #1
2:  CBNZ    x2, 1b

    // set stack pointer
    ADR     x1, _start
    MOV     sp, x1

    // load kernel
    BL      load_kernel
    // jump to kernel, this call should not return
    // absolute branch
    LDR     x1, =__kernel
    BR      x1

3:  WFE
    B       3b
