#include "mm.h"

.section ".text.boot"

.globl _start
_start:
	mov x24, x0
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	ldr	x0, =__bss_start
	ldr	x1, =__bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #LOW_MEMORY // 4MB
	mov x0, x24
	bl	bootloader_main
	b 	proc_hang		// should never come here
