.section ".text.boot"

#define CORE0_TIMER_IRQ_CTRL 0x40000040

// save general registers to stack
.macro save_all
//    sub sp, sp, 32 * 8
    sub	sp, sp, 16 * 17
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
    mrs	x28, spsr_el1
	mrs	x29, elr_el1
	stp x28, x29, [sp ,16 * 16]
.endm

// load general registers from stack
.macro load_all
    ldp x28, x29, [sp ,16 * 16]
	msr	spsr_el1, x28
	msr	elr_el1, x29
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
//    add sp, sp, 32 * 8
    add 	sp, sp, 16 * 17
.endm

.global _start
_start: 
    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
1:  ldr     x1, =_start
    
    bl 	    from_el2_to_el1
    mov     sp, x1

    ldr     x0, =exception_vector_table
    msr     vbar_el1, x0

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
2:  cbz     w2, 3f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 2b

3:
    // jump to C code, should not return  
    bl      main
    // for failsafe, halt this core too
    b       1b

exception_handler:
    save_all
    mrs     x0, esr_el1
    mrs     x1, elr_el1
    mrs     x2, spsr_el1
    bl 	    exc_handler
    load_all
    eret

__irq_handler:
	save_all
	bl	irq_handler
	load_all
	eret

.align 11 // vector table should be aligned to 0x800
exception_vector_table:
    b exception_handler // branch to a handler function.
    .align 7 // entry size is 0x80, .align will pad 0
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
    b exception_handler
    .align 7

    b exception_handler    
    .align 7
    b __irq_handler
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

from_el2_to_el1:
    mov x0, #(1 << 31) // EL1 uses aarch64
    msr hcr_el2, x0
    mov x0, #0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1
