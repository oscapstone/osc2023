
.section ".text"

.globl from_el2_to_el1
.globl set_exception_vector_table
.globl exception_vector_table
.globl core_timer_enable

.macro vt_entry label
    .align 7
    b \label
.endm

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

.align 11 // vector table should be aligned to 0x800
exception_vector_table:
    vt_entry	exception_handler			// Synchronous EL1t
	vt_entry	exception_handler			// IRQ EL1t
	vt_entry	exception_handler			// FIQ EL1t
	vt_entry	exception_handler			// Error EL1t

	vt_entry	exception_handler			// Synchronous EL1h
	vt_entry	irq_EL1h_handler		    // IRQ EL1h
	vt_entry	exception_handler			// FIQ EL1h
	vt_entry	exception_handler			// Error EL1h

	vt_entry	exception_handler			// Synchronous 64-bit EL0
	vt_entry	exception_handler			// IRQ 64-bit EL0
	vt_entry	exception_handler			// FIQ 64-bit EL0
	vt_entry	exception_handler   		// Error 64-bit EL0

	vt_entry	exception_handler			// Synchronous 32-bit EL0
	vt_entry	exception_handler			// IRQ 32-bit EL0
	vt_entry	exception_handler			// FIQ 32-bit EL0
	vt_entry	exception_handler   		// Error 32-bit EL0

exception_handler:
    save_all
    bl exception_entry
    load_all
    eret

irq_EL1h_handler:
    save_all
    bl irq_exc_router
    load_all
    eret

set_exception_vector_table:
    adr x0, exception_vector_table  
    msr vbar_el1, x0
    ret

from_el2_to_el1:
    mov x0, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1
