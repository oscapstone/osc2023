#include "mm.h"

.section ".text.boot"

.globl _start
_start:
	mov x19, x0

	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU, cbz: conditional branch if zero
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	// Initialize bss
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	//Initialize stack
	ldr x0, =_stack_top
	mov	sp, x0 

	mov x0,x19
	bl	kernel_main
	b 	proc_hang		// should never come here