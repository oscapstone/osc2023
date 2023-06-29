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
