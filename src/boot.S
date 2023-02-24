#include "mm.h"
//put .text.boot as the first symbol in data section
.section ".text.boot"

//.global makes the symbol visible to ld.
.global _start
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
  //clear bss section
  //my_bzero(__bss_start, size), a function defined in mm.S
	adr	x0, __bss_start
	adr	x1, __bss_end
	sub	x1, x1, x0
	bl 	my_bzero

  //move sp to proper location
	mov	sp, #LOW_MEMORY 
	bl	main        // branch PC to main function which is defined in main.c
	b 	proc_hang		// should never come here