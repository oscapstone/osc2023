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
	ldr	x1, =_start	
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
	// Jump to main
	bl 	main
	b 	1b

