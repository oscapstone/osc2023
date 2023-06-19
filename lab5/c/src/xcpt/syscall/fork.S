.section ".text"

sys_fork:
    str lr, [sp, -16]!

    // Save integer context.
    mrs x0, tpidr_el1
    stp x19, x20, [x0, 16 + 0 * 16]
    stp x21, x22, [x0, 16 + 1 * 16]
    stp x23, x24, [x0, 16 + 2 * 16]
    stp x25, x26, [x0, 16 + 3 * 16]
    stp x27, x28, [x0, 16 + 4 * 16]
    str x29, [x0, 16 + 5 * 16]
    mrs x1, sp_el0
    str x1, [x0, 16 + 6 * 16 + 8]

    // Save FP/SIMD context.
    ldr x0, [x0, 16 + 7 * 16]
    stp q0, q1, [x0, 0 * 32]
    stp q2, q3, [x0, 1 * 32]
    stp q4, q5, [x0, 2 * 32]
    stp q6, q7, [x0, 3 * 32]
    stp q8, q9, [x0, 4 * 32]
    stp q10, q11, [x0, 5 * 32]
    stp q12, q13, [x0, 6 * 32]
    stp q14, q15, [x0, 7 * 32]
    stp q16, q17, [x0, 8 * 32]
    stp q18, q19, [x0, 9 * 32]
    stp q20, q21, [x0, 10 * 32]
    stp q22, q23, [x0, 11 * 32]
    stp q24, q25, [x0, 12 * 32]
    stp q26, q27, [x0, 13 * 32]
    stp q28, q29, [x0, 14 * 32]
    stp q30, q31, [x0, 15 * 32]
    add x0, x0, 16 * 32
    mrs x1, fpcr
    mrs x2, fpsr
    stp x1, x2, [x0]

    add x0, sp, 2 * 16
    bl sys_fork_impl

    ldr lr, [sp], 16
    ret

.type sys_fork, function
.size sys_fork, . - sys_fork
.global sys_fork
