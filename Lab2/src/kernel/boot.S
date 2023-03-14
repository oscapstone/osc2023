#include "mm.h"

.section ".text.boot"

.globl _start
_start:
	ldr x1, =_dtb_ptr
    str x0, [x1]
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	adr	x0, __bss_start
	adr	x1, __bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #LOW_MEMORY // 4MB
	bl	kernel_main
	b 	proc_hang		// should never come here

.global _dtb_ptr
.section .data
_dtb_ptr: .dc.a 0x0
