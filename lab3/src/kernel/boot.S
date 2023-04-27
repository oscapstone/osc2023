#include "mm.h"

.section ".text.boot"

.globl _start

_start:
	
	mrs	x0, mpidr_el1		
	and	x0, x0, #0xFF		// Check processor id
	cbz	x0, master			// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	bl	from_el2_to_el1		// Switch from EL2 to EL1
	bl	set_exception_vector_table

	// Initialize bss
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero				// Accept two arguments: start address(bss_begin), section size(bss_end - bss_begin)
	
	//Initialize stack
	//mov	sp, #LOW_MEMORY 	// Set stack pointer(4MB), LOW_MEMORY is defined in mm.h
	ldr x0, =stack_top
	mov	sp, x0 

	bl	main				// "main" is defined in main.c
	b 	proc_hang			// !!! never come here !!!













/*
mrs: load value from system register to general register
and: "And" logical operator
cbz: branch if equal to zero
b:   branch
adr: load relative address from label to register
sub: subtract two registers' value
bl:  branch and link, store return address
mov: move value from constant or register to register
*/
