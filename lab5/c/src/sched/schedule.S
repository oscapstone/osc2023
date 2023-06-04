.section ".text"

schedule:
    mrs x0, tpidr_el1

    stp x19, x20, [x0, 16 + 0 * 16]
    stp x21, x22, [x0, 16 + 1 * 16]
    stp x23, x24, [x0, 16 + 2 * 16]
    stp x25, x26, [x0, 16 + 3 * 16]
    stp x27, x28, [x0, 16 + 4 * 16]
    stp x29, x30, [x0, 16 + 5 * 16]
    mov x1, sp
    str x1, [x0, 16 + 6 * 16]

    bl _sched_pick_thread

    // Fallthrough.

.size schedule, . - schedule
.type schedule, function
.global schedule

_sched_run_thread:
    ldp x19, x20, [x0, 16 + 0 * 16]
    ldp x21, x22, [x0, 16 + 1 * 16]
    ldp x23, x24, [x0, 16 + 2 * 16]
    ldp x25, x26, [x0, 16 + 3 * 16]
    ldp x27, x28, [x0, 16 + 4 * 16]
    ldp x29, x30, [x0, 16 + 5 * 16]
    ldr x1, [x0, 16 + 6 * 16]
    mov sp, x1

    msr tpidr_el1, x0

    ret

.size _sched_run_thread, . - _sched_run_thread
.type _sched_run_thread, function
.global _sched_run_thread
