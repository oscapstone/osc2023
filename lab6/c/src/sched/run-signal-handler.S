#include "oscos/utils/save-ctx.S"

.section ".text"

run_signal_handler:
    // Save integer context.

    stp x19, x20, [sp, -(12 * 8)]!
    stp x21, x22, [sp, 2 * 8]
    stp x23, x24, [sp, 4 * 8]
    stp x25, x26, [sp, 6 * 8]
    stp x27, x28, [sp, 8 * 8]
    stp x29, lr, [sp, 10 * 8]

    // Save FP/SIMD context.

    mrs x2, fpcr
    mrs x3, fpsr
    stp x2, x3, [sp, -16]!

    stp q0, q1, [sp, -(32 * 16)]!
    stp q2, q3, [sp, 2 * 16]
    stp q4, q5, [sp, 4 * 16]
    stp q6, q7, [sp, 6 * 16]
    stp q8, q9, [sp, 8 * 16]
    stp q10, q11, [sp, 10 * 16]
    stp q12, q13, [sp, 12 * 16]
    stp q14, q15, [sp, 14 * 16]
    stp q16, q17, [sp, 16 * 16]
    stp q18, q19, [sp, 18 * 16]
    stp q20, q21, [sp, 20 * 16]
    stp q22, q23, [sp, 22 * 16]
    stp q24, q25, [sp, 24 * 16]
    stp q26, q27, [sp, 26 * 16]
    stp q28, q29, [sp, 28 * 16]
    stp q30, q31, [sp, 30 * 16]

    // - Unmask all interrupts.
    // - AArch64 execution state.
    // - EL0t.
    msr spsr_el1, xzr

    adr x2, sig_handler_main
    msr elr_el1, x2

    // Zero the integer registers except for x0, which is used to pass an
    // argument to sig_handler_main.

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

.type run_signal_handler, function
.size run_signal_handler, . - run_signal_handler
.global run_signal_handler
