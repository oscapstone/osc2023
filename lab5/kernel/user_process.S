.global get_current_process
get_current_process:
        mrs     x0, tpidr_el0
        ret

.global set_current_process
set_current_process:
        msr     tpidr_el0, x0
        ret

/*
 * void user_proc_context_switch
 *      (struct el0_context *prev, struct el0_context *next)
 */
.global user_proc_context_switch
user_proc_context_switch:
        stp     x19, x20, [x0, 16 * 0]
        stp     x21, x22, [x0, 16 * 1]
        stp     x23, x24, [x0, 16 * 2]
        stp     x25, x26, [x0, 16 * 3]
        stp     x27, x28, [x0, 16 * 4]
        stp     fp, lr, [x0, 16 * 5]
        mov     x9, sp
        mrs     x10, elr_el1
        stp     x9, x10, [x0, 16 * 6]
        mrs     x9, sp_el0
        ; mrs     x10, esr_el1
        str     x9, [x0, 16 * 7]

        ldp     x19, x20, [x1, 16 * 0]
        ldp     x21, x22, [x1, 16 * 1]
        ldp     x23, x24, [x1, 16 * 2]
        ldp     x25, x26, [x1, 16 * 3]
        ldp     x27, x28, [x1, 16 * 4]
        ldp     fp, lr, [x1, 16 * 5]
        ldp     x9, x10, [x1, 16 * 6]
        mov     sp,  x9
        msr     elr_el1, x10
        ldr     x9, [x1, 16 * 7]
        msr     sp_el0, x9
        ; msr     esr_el1, x10

        msr     tpidr_el1, x1
        ret

.global load_process_context
load_process_context:
        ldp     x19, x20, [x0, 16 * 0]
        ldp     x21, x22, [x0, 16 * 1]
        ldp     x23, x24, [x0, 16 * 2]
        ldp     x25, x26, [x0, 16 * 3]
        ldp     x27, x28, [x0, 16 * 4]
        ldp     fp, lr, [x0, 16 * 5]
        ldp     x9, x10, [x0, 16 * 6]
        mov     sp,  x9
        msr     elr_el1, x10
        ldr     x9, [x0, 16 * 7]
        msr     sp_el0, x9
        ; msr     esr_el1, x10

        msr     tpidr_el1, x0
        ret