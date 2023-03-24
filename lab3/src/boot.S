.section ".text.boot"

.global _start

_start:
    bl from_el2_to_el1
    bl set_exception_vector_table
	bl core_timer_enable
    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    adr     x1, _start
    mov     sp, x1

    // clear bss
    adr     x1, __bss_start
    ldr     w2, =__bss_size
1:  cbz     w2, 2f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 1b

2:  // jump to C code, should not return
    bl      kernel_main
    // for failsafe, halt this core too
3:  wfe
    b       3b

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

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

exception_handler:
    save_all
    bl exception_entry
    load_all
    eret

timer_handler:
    save_all
    bl core_timer_handler
    load_all
	eret

.align 11 // vector table should be aligned to 0x800, which 11 is the exp(2, 11), and 8*256 is 1024
.global exception_vector_table
exception_vector_table:
// SERROR -> FIQ -> IRQ -> Synchronous

// Exception from a lower EL and lower ELs are AARCH32
    b exception_handler // branch to a handler function.
    .align 7 // entry size is 0x80, .align will pad 0
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
// Exception from a lower EL and at least one lower EL is AARCH64
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
// Exception from the current EL while using SP_ELx
    b exception_handler
    .align 7
    b timer_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
// Exception from the current EL while using SP_EL0
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7
    b exception_handler
    .align 7

set_exception_vector_table:
// Load the address of lable exception_vector_table
    adr x0, exception_vector_table
// move the address of exception_vector_table to the status register
    msr vbar_el1, x0
    ret

.equ CORE0_TIMER_IRQ_CTRL, 0x40000040

core_timer_enable:
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mrs x0, cntfrq_el0
    msr cntp_tval_el0, x0 // set expired time
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
	ret

core_timer_handler:
    mrs x0, cntfrq_el0
	mrs x1, cntpct_el0
	stp lr, x0, [sp, #-16]!
	bl print_core_timer
	ldp lr, x0, [sp], #16
	lsl x0, x0, #1
	msr cntp_tval_el0, x0
	ret
