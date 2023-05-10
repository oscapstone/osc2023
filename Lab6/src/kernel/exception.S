#define CORE0_INTERRUPT_SOURCE 0x40000060
#define IIR_ADDR 0x3f215048

.global set_el1_exception_vector_table
.global exception_vector_table
.global enable_interrupt
.global disable_interrupt

enable_interrupt:
    msr DAIFClr, 0xf
    ret

disable_interrupt:
    msr DAIFSet, 0xf
    ret

// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 8
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
.endm

.macro save_all_sys
    sub sp, sp, 32 * 9
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    mrs x0, spsr_el1
    mrs x1, elr_el1
    mrs x2, sp_el0
    stp x30, x0, [sp ,16 * 15]
    stp x1, x2, [sp ,16 * 16]
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm

.macro load_all_sys
    ldp x1, x2, [sp ,16 * 16]
    ldp x30, x0, [sp ,16 * 15]
    msr spsr_el1, x0
    msr elr_el1, x1
    msr sp_el0, x2
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    add sp, sp, 32 * 9
.endm

.align 11 // vector table should be aligned to 0x800
el1_exception_vector_table:
    b exception_handler // branch to a handler function.
    .align 7 // entry size is 0x80, .align will pad 0
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    b exception_handler_sync_sp_elx_same_el
    .align 7
    b exception_handler_irq_sp_elx_same_el
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    b exception_handler_sync_sp_elx_lower_el
    .align 7
    b exception_handler_irq_sp_elx_lower_el
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

set_el1_exception_vector_table:
    adr x0, el1_exception_vector_table
    msr vbar_el1, x0
    ret lr

exception_handler:
    save_all
    mov x0, #0
    mrs x1, esr_el1
    mrs x2, elr_el1
    mrs x3, spsr_el1
    mrs x4, far_el1
    bl exc_handler
    load_all
    eret

exception_handler_sync_sp_elx_lower_el:
    save_all_sys
    mov x0, sp
    bl el0_to_el1_sync_handler
    load_all_sys
    eret

exception_handler_irq_sp_elx_lower_el:
    b el0_core_timer_handler

exception_handler_sync_sp_elx_same_el:
    b exception_handler

exception_handler_irq_sp_elx_same_el:
    b el1_core_interrupt_handler

el0_core_timer_handler:
    save_all_sys
    mrs x0, cntpct_el0
    mrs x1, cntfrq_el0    
    bl el0_timer_handler
    load_all_sys
    eret

el1_core_interrupt_handler:
    save_all_sys
    bl el1_irq_interrupt_handler
    load_all_sys
    eret