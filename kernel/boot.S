#include "mm.h"
//put .text.boot as the first symbol in text section
.section ".text.boot"
//.global makes the symbol visible to ld.
.global _start
_start:
	ldr    x1, =_dtb_ptr // bootloader put dtb address on x1 and wake kernel up
    str    x0, [x1]
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
  //clear bss section
  //my_bzero(__bss_start, size), a function defined in mm.S
  	bl from_el2_to_el1
	adr	x0, __bss_start
	adr	x1, __bss_end
	sub	x1, x1, x0
	bl 	my_bzero

  //move sp to proper location
	mov	sp, #LOW_MEMORY 
	bl	main        // branch PC to main function which is defined in main.c
	b 	proc_hang		// should never come here

.global _dtb_ptr
.section .data
_dtb_ptr: .dc.a 0x0