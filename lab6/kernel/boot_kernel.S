#include "mmu.h"
#include "peripherals/base.h"

.section ".text.boot"

.globl _start
_start:
    mrs    x0, mpidr_el1        
    and    x0, x0, #3        // Check processor id
    cbnz   x0, proc_hang     // Hang for all non-primary CPU
    bl     from_el2_to_el1
    bl     set_exception_vector_table

master:
    mov    x2, #VA_START
    ldr    x0, =__bss_begin
    sub    x0, x0, x2
    ldr    x1, =__bss_end
    sub    x1, x1, x2
    sub    x1, x1, x0

memzero:
    cbz    x1, mmu
	str    xzr, [x0], #8
	subs   x1, x1, #8
	cbnz   x1, memzero

mmu:
    mov    x0, #VA_START
    add    sp, x0, #0x80000

    bl     set_tcr
    bl     set_mair
    bl     make_page_tables

    mov    x0, #SCTLR_MMU_ENABLED
    msr    sctlr_el1, x0

    ldr    x0, =kernel_main
    br     x0

proc_hang: 
    wfe
    b proc_hang

set_exception_vector_table:
    ldr x0, =exception_vector_table  
    msr vbar_el1, x0
    ret

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

set_tcr:
    ldr x0, =TCR_CONFIG_DEFAULT
    msr tcr_el1, x0
    ret

set_mair:
    ldr x0, =MAIR_VALUE
    msr mair_el1, x0
    ret

make_page_tables:
    ldr x0, =pg_dir
    and x0, x0, #VA_MASK
    add x1, x0, 0x1000

    ldr x2, =BOOT_PGD_ATTR
    orr x2, x1, x2
    str x2, [x0]

    ldr x2, =BOOT_PUD_ATTR
    mov x3, 0x00000000
    orr x3, x2, x3
    str x3, [x1]
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8]

    msr ttbr0_el1, x0
    msr ttbr1_el1, x0
    ret