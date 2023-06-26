#define TCR_T0SZ_POSN 0
#define TCR_T0SZ_REGION_48BIT ((64 - 48) << TCR_T0SZ_POSN)
#define TCR_TG0_POSN 14
#define TCR_TG0_4KB (0b00 << TCR_TG0_POSN)
#define TCR_T1SZ_POSN 16
#define TCR_T1SZ_REGION_48BIT ((64 - 48) << TCR_T1SZ_POSN)
#define TCR_TG1_POSN 30
#define TCR_TG1_4KB (0b10 << TCR_TG1_POSN)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IX_DEVICE_nGnRnE 0
#define MAIR_IX_NORMAL_NOCACHE 1

.section ".text._start"

_start:
    // Enable FP/SIMD.
    mov x1, 0x300000
    msr cptr_el2, x1

    // Switch to EL1.

    // - Use AArch64 in EL1.
    // - Trap nothing to EL2.
    // - Disable interrupt routing to EL2.
    // - Disable stage 2 address translation.
    mov x1, 0x80000000
    msr hcr_el2, x1

    // - Mask all interrupts.
    // - AArch64 execution state.
    // - EL1h. (Uses SP_EL1).
    mov x1, 0x3c5
    msr spsr_el2, x1

    adr x1, .Lin_el1
    msr elr_el2, x1
    eret

.Lin_el1:
    // Setup virtual memory.

    ldr x1, \
        =(TCR_T0SZ_REGION_48BIT | TCR_TG0_4KB | TCR_T1SZ_REGION_48BIT \
            | TCR_TG1_4KB)
    msr tcr_el1, x1

    ldr x1, \
        =((MAIR_DEVICE_nGnRnE << (MAIR_IX_DEVICE_nGnRnE * 8)) \
            | (MAIR_NORMAL_NOCACHE << (MAIR_IX_NORMAL_NOCACHE * 8)))
    msr mair_el1, x1

    ldr x1, =kernel_pgd
    ldr x2, =_kernel_vm_base
    sub x1, x1, x2

    ldr x3, [x1]
    ldr x4, =kernel_pud
    sub x4, x4, x2
    orr x3, x3, x4
    str x3, [x1]

    msr ttbr0_el1, x1
    msr ttbr1_el1, x1

    mrs x1, sctlr_el1
    orr x1, x1, 1
    msr sctlr_el1, x1

    ldr x1, =.Lstart_after_mmu
    br x1

.Lstart_after_mmu:
    // Make the devicetree address virtual.
    add x0, x0, x2

    // Enable FP/SIMD.
    mov x1, 0x300000
    msr cpacr_el1, x1

    // Allow user programs to access the timer.
    mov x1, 0x1
    msr cntkctl_el1, x1

    msr tpidr_el1, xzr

    // Clear the .bss section.

    ldr x1, =_sbss
    ldr x2, =_ebss
    b .Lclear_bss_loop_test

.Lclear_bss_loop_body:
    // This does not generate unaligned memory accesses, since the .bss section
    // is 16-byte aligned. See `linker.ld`.
    stp xzr, xzr, [x1], 16

.Lclear_bss_loop_test:
    cmp x1, x2
    b.ne .Lclear_bss_loop_body

    // Set the stack pointer.

    ldr x1, =_estack
    mov sp, x1

    // Call the main function.

    bl main

    // Park the core.

.Lpark_loop:
    wfe
    b .Lpark_loop

.size _start, . - _start
.type _start, function
.global _start
