.section ".text"

xcpt_svc_handler:
    // Save lr.
    str lr, [sp, -32]!

    // Save spsr_el1 and elr_el1 since they can be clobbered by other ISRs.
    mrs x10, spsr_el1
    mrs x11, elr_el1
    stp x10, x11, [sp, 16]

    // Unmask interrupts.
    msr daifclr, 0xf

    // Check the system call number.
    ubfx x9, x9, 0, 16
    cbnz x9, .Lenosys
    cmp x8, 21
    b.hi .Lenosys

    // Table-jump to the system call function.
    adr lr, .Lend
    adr x9, syscall_table
    add x9, x9, x8, lsl 2
    br x9

.Lenosys:
    bl sys_enosys

.Lend:
    // Store the result into the trap frame.
    str x0, [sp, 32]

    // Mask the interrupt since we don't want ISRs to clobber spsr_el1 and
    // esr_el1 while we're restoring them.
    msr daifset, 0xf

    // Restore spsr_el1 and elr_el1.
    ldp x0, x1, [sp, 16]
    msr spsr_el1, x0
    msr elr_el1, x1

    // Restore lr.
    ldr lr, [sp], 32

    ret

.type xcpt_svc_handler, function
.size xcpt_svc_handler, . - xcpt_svc_handler
.global xcpt_svc_handler
