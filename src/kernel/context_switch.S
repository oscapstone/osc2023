.global context_switch_to
context_switch_to:
    str x19, [x0, 16 * 9 + 8]
    stp x20, x21, [x0, 16 * 10]
    stp x22, x23, [x0, 16 * 11]
    stp x24, x25, [x0, 16 * 12]
    stp x26, x27, [x0, 16 * 13]
    stp x28, fp, [x0, 16 * 14]
    mov x3, sp
    mrs x12, ttbr0_el1
    stp lr, x3, [x0, 16 * 15]
    str x12, [x0, 16 * 16]

    ldr x12, [x1, 16 * 16]
    ldr x19, [x1, 16 * 9 + 8]
    ldp x20, x21, [x1, 16 * 10]
    ldp x22, x23, [x1, 16 * 11]
    ldp x24, x25, [x1, 16 * 12]
    ldp x26, x27, [x1, 16 * 13]
    ldp x28, fp, [x1, 16 * 14]
    ldp lr, x3, [x1, 16 * 15]
    mov sp,  x3
    msr tpidr_el1, x2

    dsb ish
    msr ttbr0_el1, x12
    tlbi vmalle1is
    dsb ish
    isb
    ret

.global thread_get_current
thread_get_current:
    mrs x0, tpidr_el1
    ret