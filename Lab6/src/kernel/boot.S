#include "mm.h"

.global _dtb_ptr
.section .data
_dtb_ptr: .dc.a 0x0

.section ".text.boot"

#define PA(p) (p - 0xffff000000000000)

.globl _start
_start:
	cbz x24, x24c	// Check if bootloader, see bootloader's boot.S for more info
x24nc:
	ldr x21, =PA(_dtb_ptr)
    str x24, [x21]
	b	go
x24c:
	ldr x21, =PA(_dtb_ptr)
    str x0, [x21]
go:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	mrs x0, cpacr_el1
	orr x0, x0, #(3 << 20)
	msr cpacr_el1, x0

	// get CurrentEL
    mrs x0, CurrentEL
    and x0, x0, #12 // clear reserved bits

	// running at EL3? branch if in EL3
    cmp x0, #12
    beq from_el3_to_el2
    
	// running at EL1? branch if in EL1
	cmp x0, #4
	beq bss

    bl from_el2_to_el1

bss:
	ldr	x0, =PA(__bss_start)
	ldr	x1, =PA(__bss_end)
	sub	x1, x1, x0
	bl 	memzero
	
	bl tcr_init
	bl mair_init
	bl setup_kernel_space_mapping
	bl setup_identity_mapping

	bl set_el1_exception_vector_table	// set el1 exception vector table base

	ldr x9, =PA(_kernel_end)
    mov sp, x9
	bl	kernel_main
	b 	proc_hang		// should never come here

from_el3_to_el2:
	mov x2, #0x5b1
	msr scr_el3, x2
	mov x2, #0x3c9
	msr spsr_el3, x2
	adr x2, from_el2_to_el1
	msr elr_el3, x2
	eret

from_el2_to_el1:
	msr	sp_el1, x1
	// enable CNTP for EL1
	mrs x0, cnthctl_el2
	orr x0, x0, #3
	msr cnthctl_el2, x0
	msr cntvoff_el2, xzr
	// enable AArch64 in EL1
	mov x0, #(1 << 31)		// AArch64
	orr x0, x0, #(1 << 1)	// SWIO hardwired on Pi3
	msr hcr_el2, x0
	mrs x0, hcr_el2
	// change execution level to EL1
	mov x2, #0x3c5
	msr spsr_el2, x2
	msr elr_el2, lr
	eret