.section ".text"

user_program_main:
    // We don't need to synchronize the data cache and the instruction cache
    // since we haven't enabled any of them. Nonetheless, we still have to
    // synchronize the fetched instruction stream using the `isb` instruction.
    isb

    // Mask the interrupt to prevent spsr_el1 and elr_el1 from being clobbered
    // by ISRs.
    msr daifset, 0xf

    msr elr_el1, x0
    msr sp_el0, x1
    mov sp, x2

    // - Unask all interrupts.
    // - AArch64 execution state.
    // - EL0t.
    msr spsr_el1, xzr

    // Zero the integer registers.
    mov x0, 0
    mov x1, 0
    mov x2, 0
    mov x3, 0
    mov x4, 0
    mov x5, 0
    mov x6, 0
    mov x7, 0
    mov x8, 0
    mov x9, 0
    mov x10, 0
    mov x11, 0
    mov x12, 0
    mov x13, 0
    mov x14, 0
    mov x15, 0
    mov x16, 0
    mov x17, 0
    mov x18, 0
    mov x19, 0
    mov x20, 0
    mov x21, 0
    mov x22, 0
    mov x23, 0
    mov x24, 0
    mov x25, 0
    mov x26, 0
    mov x27, 0
    mov x28, 0
    mov x29, 0
    mov lr, 0

    // Zero the FP/SIMD registers.
    movi d0, 0
    movi d1, 0
    movi d2, 0
    movi d3, 0
    movi d4, 0
    movi d5, 0
    movi d6, 0
    movi d7, 0
    movi d8, 0
    movi d9, 0
    movi d10, 0
    movi d11, 0
    movi d12, 0
    movi d13, 0
    movi d14, 0
    movi d15, 0
    movi d16, 0
    movi d17, 0
    movi d18, 0
    movi d19, 0
    movi d20, 0
    movi d21, 0
    movi d22, 0
    movi d23, 0
    movi d24, 0
    movi d25, 0
    movi d26, 0
    movi d27, 0
    movi d28, 0
    movi d29, 0
    movi d30, 0
    movi d31, 0

    eret

.type user_program_main, function
.size user_program_main, . - user_program_main
.global user_program_main
