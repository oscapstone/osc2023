#include "oscos/utils/save-ctx.S"

.section ".text"

sys_sigreturn:
    // Discard the trap frame.

    add sp, sp, 12 * 16

    // Check if the system call is called within a signal handler context.

    bl sys_sigreturn_check

    // Restore FP/SIMD context.

    ldp q30, q31, [sp, 30 * 16]
    ldp q28, q29, [sp, 28 * 16]
    ldp q26, q27, [sp, 26 * 16]
    ldp q24, q25, [sp, 24 * 16]
    ldp q22, q23, [sp, 22 * 16]
    ldp q20, q21, [sp, 20 * 16]
    ldp q18, q19, [sp, 18 * 16]
    ldp q16, q17, [sp, 16 * 16]
    ldp q14, q15, [sp, 14 * 16]
    ldp q12, q13, [sp, 12 * 16]
    ldp q10, q11, [sp, 10 * 16]
    ldp q8, q9, [sp, 8 * 16]
    ldp q6, q7, [sp, 6 * 16]
    ldp q4, q5, [sp, 4 * 16]
    ldp q2, q3, [sp, 2 * 16]
    ldp q0, q1, [sp], (32 * 16)

    ldp x0, x1, [sp], 16
    mrs x0, fpcr
    mrs x1, fpsr

    // Restore integer context.

    ldp x29, lr, [sp, 10 * 8]
    ldp x27, x28, [sp, 8 * 8]
    ldp x25, x26, [sp, 6 * 8]
    ldp x23, x24, [sp, 4 * 8]
    ldp x21, x22, [sp, 2 * 8]
    ldp x19, x20, [sp], (12 * 8)

    ret

.type sys_sigreturn, function
.size sys_sigreturn, . - sys_sigreturn
.global sys_sigreturn
