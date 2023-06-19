.section ".text"

suspend_to_wait_queue:
    // Enter critical section.

    msr daifset, 0xf

    // Get current thread instance.

    mrs x2, tpidr_el1

    // Save integer context.

    stp x19, x20, [x2, 16 + 0 * 16]
    stp x21, x22, [x2, 16 + 1 * 16]
    stp x23, x24, [x2, 16 + 2 * 16]
    stp x25, x26, [x2, 16 + 3 * 16]
    stp x27, x28, [x2, 16 + 4 * 16]
    stp x29, x30, [x2, 16 + 5 * 16]
    mov x1, sp
    mrs x3, sp_el0
    stp x1, x3, [x2, 16 + 6 * 16]

    // Save FP/SIMD context, if there is one.

    ldr x1, [x2, 16 + 7 * 16]
    cbz x1, .Lafter_save_fp_ctx

    stp q0, q1, [x1, 0 * 32]
    stp q2, q3, [x1, 1 * 32]
    stp q4, q5, [x1, 2 * 32]
    stp q6, q7, [x1, 3 * 32]
    stp q8, q9, [x1, 4 * 32]
    stp q10, q11, [x1, 5 * 32]
    stp q12, q13, [x1, 6 * 32]
    stp q14, q15, [x1, 7 * 32]
    stp q16, q17, [x1, 8 * 32]
    stp q18, q19, [x1, 9 * 32]
    stp q20, q21, [x1, 10 * 32]
    stp q22, q23, [x1, 11 * 32]
    stp q24, q25, [x1, 12 * 32]
    stp q26, q27, [x1, 13 * 32]
    stp q28, q29, [x1, 14 * 32]
    stp q30, q31, [x1, 15 * 32]
    add x1, x1, 16 * 32
    mrs x3, fpcr
    mrs x4, fpsr
    stp x3, x4, [x1]

.Lafter_save_fp_ctx:
    mov x1, x0
    mov x0, x2
    bl _sched_move_thread_to_queue_and_pick_thread

    // Fallthrough.

.size suspend_to_wait_queue, . - suspend_to_wait_queue
.type suspend_to_wait_queue, function
.global suspend_to_wait_queue

_sched_run_thread:
    // Restore integer context.

    ldp x19, x20, [x0, 16 + 0 * 16]
    ldp x21, x22, [x0, 16 + 1 * 16]
    ldp x23, x24, [x0, 16 + 2 * 16]
    ldp x25, x26, [x0, 16 + 3 * 16]
    ldp x27, x28, [x0, 16 + 4 * 16]
    ldp x29, x30, [x0, 16 + 5 * 16]
    ldp x1, x2, [x0, 16 + 6 * 16]
    mov sp, x1
    msr sp_el0, x2

    // Restore FP/SIMD context, if there is one.

    ldr x1, [x0, 16 + 7 * 16]
    cbz x1, .Lafter_load_fp_ctx

    ldp q0, q1, [x1, 0 * 32]
    ldp q2, q3, [x1, 1 * 32]
    ldp q4, q5, [x1, 2 * 32]
    ldp q6, q7, [x1, 3 * 32]
    ldp q8, q9, [x1, 4 * 32]
    ldp q10, q11, [x1, 5 * 32]
    ldp q12, q13, [x1, 6 * 32]
    ldp q14, q15, [x1, 7 * 32]
    ldp q16, q17, [x1, 8 * 32]
    ldp q18, q19, [x1, 9 * 32]
    ldp q20, q21, [x1, 10 * 32]
    ldp q22, q23, [x1, 11 * 32]
    ldp q24, q25, [x1, 12 * 32]
    ldp q26, q27, [x1, 13 * 32]
    ldp q28, q29, [x1, 14 * 32]
    ldp q30, q31, [x1, 15 * 32]
    add x1, x1, 16 * 32
    ldp x2, x3, [x1]
    msr fpcr, x2
    msr fpcr, x3

.Lafter_load_fp_ctx:
    msr tpidr_el1, x0

    // Unmask interrupts.

    msr daifclr, 0xf

    ret

.size _sched_run_thread, . - _sched_run_thread
.type _sched_run_thread, function
.global _sched_run_thread
