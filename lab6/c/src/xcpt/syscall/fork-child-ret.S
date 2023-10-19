.section ".text"

fork_child_ret:
    // Mask the interrupt to prevent spsr_el1 and elr_el1 from being clobbered
    // by ISRs.
    msr daifset, 0xf

    ldp x0, x1, [sp]
    msr spsr_el1, x0
    msr elr_el1, x1

    mov x0, 0
    ldr x1, [sp, 16 + 0 * 16 + 8]
    ldp x2, x3, [sp, 16 + 1 * 16]
    ldp x4, x5, [sp, 16 + 2 * 16]
    ldp x6, x7, [sp, 16 + 3 * 16]
    ldp x8, x9, [sp, 16 + 4 * 16]
    ldp x10, x11, [sp, 16 + 5 * 16]
    ldp x12, x13, [sp, 16 + 6 * 16]
    ldp x14, x15, [sp, 16 + 7 * 16]
    ldp x16, x17, [sp, 16 + 8 * 16]
    ldp x18, lr, [sp, 16 + 9 * 16]

    add sp, sp, 16 + 10 * 16
    eret

.type fork_child_ret, function
.size fork_child_ret, . - fork_child_ret
.global fork_child_ret
