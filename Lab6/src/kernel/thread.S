.global switch_to
switch_to:
    stp x19, x20, [x0, 16 * 0]  // curr->x19=x19, curr->x20=x20
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp,  lr,  [x0, 16 * 5]
    mov x9,  sp                 // x9 is corruptible register, caller saved
    str x9,       [x0, 16 * 6]  // curr->sp = sp
    mrs x9,       spsr_el1            // for debug purpose
    str x9,       [x0, 16 * 6 + 8]    // for debug purpose
    mrs x9,       elr_el1             // for debug purpose
    str x9,       [x0, 16 * 7]        // for debug purpose
    mrs x9,       esr_el1             // for debug purpose
    str x9,       [x0, 16 * 7 + 8]    // for debug purpose
    mrs x9,       ttbr0_el1
    str x9,       [x0, 16 * 8]  

    ldp x19, x20, [x1, 16 * 0]  // x19=next->x19=, x20=next->x20
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp,  lr,  [x1, 16 * 5]
    ldr x9,       [x1, 16 * 6]  // sp = next->sp
    mov sp,  x9
    ldr x9,       [x1, 16 * 6 + 8]    // for debug purpose
    msr spsr_el1, x9                  // for debug purpose
    ldr x9,       [x1, 16 * 7]        // for debug purpose
    msr elr_el1,  x9                  // for debug purpose
    ldr x9,       [x1, 16 * 7 + 8]    // for debug purpose
    msr esr_el1,    x9
    ldr x9,       [x1, 16 * 8]
    dsb ish
    msr ttbr0_el1, x9 // switch translation based address.
    dsb ish
    isb
    msr tpidr_el1, x1           // tpidr_el1 = (void*) next, update current thread
    ret

.global get_current
get_current:
    mrs x0, tpidr_el1
    ret

.global kernel_thread_init
kernel_thread_init:
    msr tpidr_el1, x0
    ret