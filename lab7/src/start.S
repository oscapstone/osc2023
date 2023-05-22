#include "registers.h"
#include "mmu.h"

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

    // set paging configuration (up : 0xffff000000000000 low : 0x0000000000000000)
    ldr x4, = TCR_CONFIG_DEFAULT
    msr tcr_el1, x4

    // Set Used Memory Attributes
    ldr x4, =((MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)))
    msr mair_el1, x4

    // set and enable MMU
    mov x4, 0x1000 // PGD's page frame at 0x1000
    mov x1, 0x2000 // PUD's page frame at 0x2000

    ldr x2, = BOOT_PGD_ATTR
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x4]

    ldr x2, = BOOT_PUD_ATTR
    mov x3, 0x00000000
    orr x3, x2, x3
    str x3, [x1]    // 1st 1GB mapped by the 1st entry of PUD
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

    msr ttbr0_el1, x4 // load PGD to the bottom translation-based register.
    msr ttbr1_el1, x4 // also load PGD to the upper translation based register.

    mov sp, 0x3c000000
    bl set_2M_kernel_mmu

    mrs x2, sctlr_el1
    orr x2 , x2, 1
    msr sctlr_el1, x2 // enable MMU, cache remains disabled

    // indirect branch to the upper virtual address
    ldr x2, =set_exception_vector_table
    br x2

set_exception_vector_table:
    adr x1, exception_vector_table
    msr vbar_el1, x1

    // set top of stack at 0xffff00003c000000 (last usable memory)
    movz x3, 0x0000
    movk x3, 0x3c00, lsl 16
    movk x3, 0xffff, lsl 48
    mov  sp, x3

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

    
