#include "sysregs.h"

.section ".text.boot"

.global _start
_start:
    mrs     x0, mpidr_el1
    and     x0, x0, 3
    cbnz    x0, hang_on
    
    ldr     x0, =_start
    mov     sp, x0

el2_entry:
    ldr     x1, =HCR_EL2_VALUE                  // set EL1 aarch
    msr     hcr_el2, x1

    ldr     x1, =SPSR_EL2_VALUE                 // set processor restore state after calling eret  
    msr     spsr_el2, x1

    # init sp for EL1
    mov     x0, 0x80000
    msr     sp_el1, x0

    adr     x1, el1_entry                       // eret return address
    msr     elr_el2, x1

    eret

el1_entry:
    mrs     x1, cpacr_el1
    orr     x1, x1, 0x300000                    // turn on the accessibility to SIMD and floating registers
    msr     cpacr_el1, x1

    ldr     x0, =exception_vector_table         // set up el1 exception table
    msr     vbar_el1, x0

/*
    ldr     x0, =SPSR_EL1_VALUE
    msr     spsr_el1, x0
    adr     x0, el0_entry
    msr     elr_el1, x0
    mov     x0, 0x40000
    msr     sp_el0, x0
    eret

el0_entry:
*/
    // clear bss
    ldr     x1, =_bss_start
    ldr     w2, =_bss_size

clear_loop:
    cbz     w2, pseudo_main
    str     xzr, [x1], 8
    sub     w2, w2, 1
    cbnz    w2, clear_loop

pseudo_main:
    bl      main
    b       hang_on

hang_on:
    wfe
    b       hang_on