#include "registers.h"

.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    // Switch from EL2 to EL1 .
    bl from_el2_to_el1

set_exception_vector_table:
    adr x1, exception_vector_table
    msr vbar_el1, x1

    // set top of stack at 0x3c000000 (last usable memory)
    mov     sp, 0x3c000000

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  bl      main
    // for failsafe, halt this core too
    b       1b

from_el2_to_el1:
    ldr     x1, =CPACR_EL1_VALUE     // Make el0, el1 can use Floating point and Advanced SIMD
    msr     CPACR_EL1, x1
    mov x1, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x1
    mov x1, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x1
    msr elr_el2, lr
    eret // return to EL1

    
