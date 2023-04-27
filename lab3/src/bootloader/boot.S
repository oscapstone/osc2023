#include "mm.h"

.section ".text.boot"

.globl _start

_start:
	mov x20, x0				// Save devicetree pointer to x20

	mrs	x0, mpidr_el1		
	and	x0, x0, #0xFF		// Check processor id
	cbz	x0, master			// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	// Get offset
	adr x0, _start
	ldr x1, =bootloader_start
	subs x0, x0, x1
	// If start address not equals to desired bootloader adddress, 
	// copy mem and jump to desired address
	b.ne relocate

	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero				// Accept two arguments: start address(bss_begin), section size(bss_end - bss_begin)

	//Initialize stack
	ldr x0, =stack_top
	mov	sp, x0

	; mov x0, x20
	bl	main				// "main" is defined in main.c

	b 	proc_hang			// !!! never come here !!!

relocate:
	ldr x0, =bootloader_start	// destination

	adr x1, _start				// source
	
	ldr x2, =data_end

	sub x2, x2, x0				// size
	bl memncpy

	; mov x0, x20
	ldr x1, =bootloader_start
	br x1












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
