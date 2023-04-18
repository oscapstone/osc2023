.section ".text.boot"

.global _start

_start:
	mrs	x1, mpidr_el1	// read the cpu id
	and	x1, x1, #3
	cbz	x1, 2f		// stop cpu with id > 0

1: 
	wfe
	b	1b
2:
	bl	el2_to_el1
	//ldr	x1, =_start	
	mov	x1, #0x80000 	
	mov	sp, x1		// Initialize the sp at _start
				// Note: stack is growing lower

	// Clear BSS
	ldr	x1, =__bss_start
	ldr	w2, =__bss_size
3:
	cbz	w2, 4f		// If BSS SIZE == 0, don't clear it
	str	xzr, [x1], #8
	sub	w2, w2, #1
	cbnz	w2, 3b

4:
	//bl	el2_to_el1	// Switch EL1
	//ldr	x4, =_start	// Reset stack pointer due to EL change
	//mov	sp, x4
	bl 	main		// Jump to main	
	b 	1b

el2_to_el1:
	mov 	x4, (1 << 31)	// EL1 uses aarch64, Not using x0-3
	//mov	x4, 0x80000200	// Enable UMA bit for msr in el0
	msr 	hcr_el2, x4
	mov	x4, 0x3c5	// EL1h with interrupt disable.
	//mov 	x4, 0x305
	msr	spsr_el2, x4
	msr	elr_el2, lr
	eret



