#include "mm.h"

.section ".text.boot"

.global _start
_start:
    // Preserve flattened devicetree pointer
    mov x19, x0

    // Check processor id
    mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		
	cbz	x0, master

    // Hang for all non-primary CPU, cbz: conditional branch if zero		
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	// Get offset
	adr x0, _start
	ldr x1, =_bootloader
	subs x0, x0, x1 // G_G
	// If start address not equals to desired bootloader adddress, 
	// copy mem and jump to desired address
	b.ne relocate

	// Initialize bss
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	//Initialize stack
	ldr x0, =_stack_top
	mov	sp, x0

    // call bootloader_main with device tree pointer 
    mov x0, x19
    bl	bootloader_main

	b 	proc_hang		// should never come here

relocate:
	// dest
	ldr x0, =_bootloader
	// src
	adr x1, _start
	//length
	ldr x2, =end_data

	sub x2, x2, x0
	bl memncpy

	mov x0, x19
	ldr x1, =_bootloader
	br x1

