.section ".text"

xcpt_task_queue_fini:
    mov sp, x0

    // Store x0 and x1 into the returning stack.

    ldp x0, x1, [sp, 14 * 8]
    ldr x2, [sp, 11 * 8]
    stp x0, x1, [x2, -2 * 8]

    // Restore registers except for x0 and x1.

    ldp x18, x30, [sp, 32 * 8]
    ldp x16, x17, [sp, 30 * 8]
    ldp x14, x15, [sp, 28 * 8]
    ldp x12, x13, [sp, 26 * 8]
    ldp x10, x11, [sp, 24 * 8]
    ldp x8, x9, [sp, 22 * 8]
    ldp x6, x7, [sp, 20 * 8]
    ldp x4, x5, [sp, 18 * 8]
    ldp x2, x3, [sp, 16 * 8]
    ldr x29, [sp, 10 * 8]
    ldp x27, x28, [sp, 8 * 8]
    ldp x25, x26, [sp, 6 * 8]
    ldp x23, x24, [sp, 4 * 8]
    ldp x21, x22, [sp, 2 * 8]
    ldp x19, x20, [sp]

    // Restore PSTATE and PC.

    ldp x0, x1, [sp, 12 * 8]
    msr spsr_el1, x0
    msr elr_el1, x1

    // Restore SP.

    ldr x0, [sp, 11 * 8]
    mov sp, x0

    // Restore x0 and x1.

    ldp x0, x1, [sp, -2 * 8]

    // Return.

    eret

.size xcpt_task_queue_fini, . - xcpt_task_queue_fini
.type xcpt_task_queue_fini, function
.global xcpt_task_queue_fini
