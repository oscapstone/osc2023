#include "entry.h"
#include "syscall.h"

.macro handle_invalid_entry el, type
    kernel_entry \el
    mov     x0, #\type
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    bl      show_invalid_entry_message
    b       err_hang
.endm

.macro ventry label
    .align 7
    b \label
.endm

// save general registers to stack
.macro kernel_entry, el
    sub sp, sp, #S_FRAME_SIZE
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
    
    /*The original stack pointer can be found in the sp_el0 register. 
    The value of this register must be stored and restored before and after taking an exception,
     even if we don't touch sp_el0 in the exception handler. 
     */
    .if \el == 0
    mrs x21, sp_el0
    .else
    add x21, sp, #S_FRAME_SIZE
    .endif
    /* If you don't do this you will end up having wrong value in the sp register after a context switch. */

    mrs	x22, elr_el1
	mrs	x23, spsr_el1

	stp	x30, x21, [sp, 16 * 15] 
	stp	x22, x23, [sp, 16 * 16]
.endm
/*https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson05/rpi-os.md#switching-between-el0-and-el1 */

// load general registers from stack
.macro kernel_exit, el
    ldp	x22, x23, [sp, 16 * 16]
	ldp	x30, x21, [sp, 16 * 15] 

    .if \el == 0
    msr sp_el0, x21
    .endif

	msr	elr_el1, x22			
	msr	spsr_el1, x23

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
    add sp, sp, #S_FRAME_SIZE
    eret
.endm

.align 11 // vector table should be aligned to 0x800
.globl exception_vector_table
exception_vector_table:
    ventry	sync_invalid_el1t			// Synchronous EL1t
	ventry	irq_invalid_el1t			// IRQ EL1t
	ventry	fiq_invalid_el1t			// FIQ EL1t
	ventry	error_invalid_el1t			// Error EL1t

	ventry	sync_invalid_el1h			// Synchronous EL1h
	ventry	el1_irq					    // IRQ EL1h
	ventry	fiq_invalid_el1h			// FIQ EL1h
	ventry	error_invalid_el1h			// Error EL1h

	ventry	el0_sync        			// Synchronous 64-bit EL0
	ventry	el0_irq         			// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64		// Error 64-bit EL0

	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32		// Error 32-bit EL0

sync_invalid_el1t:
	handle_invalid_entry 1, SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry 1, IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry 1, FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry 1, ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry 1, SYNC_INVALID_EL1h

el1_irq:
	kernel_entry 1
	bl	handle_irq
	kernel_exit 1

fiq_invalid_el1h:
	handle_invalid_entry 1, FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry 1, ERROR_INVALID_EL1h

el0_sync:
	kernel_entry 0
    mrs x25, esr_el1
    lsr x24, x25, #ESR_ELx_EC_SHIFT
    cmp x24, #ESR_ELx_EC_SVC64
    b.eq el0_svc
    handle_invalid_entry 0, SYNC_ERROR

el0_irq:
	kernel_entry 0
    bl  handle_irq
    kernel_exit 0

fiq_invalid_el0_64:
	handle_invalid_entry 0, FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry 0, ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry 0, SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry 0, IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry 0, FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry 0, ERROR_INVALID_EL0_32

sc_nr   .req    x25
scno    .req    x26
stbl    .req    x27

/*If exception class is equal to ESR_ELx_EC_SVC64 this means that the current exception is caused by the svc instruction 
and it is a system call. 
In this case, we jump to el0_svc label and show an error message otherwise. */

el0_svc:
    adr     stbl, sys_call_table            // load syscall table pointer
    uxtw    scno, w8                        // syscall number in w8
    mov     sc_nr, #__NR_SYSCALLS
    bl      enable_interrupt
    cmp     scno, sc_nr                     // check upper syscall limit;
    b.hs    ni_sys

    ldr     x16, [stbl, scno, lsl #3]       // address in the syscall table
    blr     x16                             // call sys_* routine
    b       ret_from_syscall
ni_sys:
    handle_invalid_entry 0, SYSCALL_ERROR
/*
Then interrupts are enabled and syscall number is compared to the total number of syscalls in the system —
 if it is greater or equal an error message is shown.
 If syscall number falls within the required range, it is used as an index in the syscall table array to obtain a pointer to the syscall handler.
 */

ret_from_syscall:
    bl      disable_interrupt               // first disables interrupts.
    str     x0, [sp, #S_X0]                 // save the value of x0 register; return value of the syscall handler, pass to user mode
    kernel_exit 0                           //because kernel_exit will restore all genral purpose register 

.globl ret_from_fork
ret_from_fork:
    bl      schedule_tail
    cbz     x19, ret_to_user                //not a kernel thread
    mov     x0, x20
    blr     x19
ret_to_user:
    bl      disable_interrupt
    kernel_exit 0

.globl err_hang
err_hang: b err_hang