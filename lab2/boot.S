.section ".text.boot"

.global _start

_start:
	//mrs	x1, mpidr_el1	// read the cpu id
	mov	sp, #0x30000	// Set stack to 0x30000		
				// Note: stack is growing lower
	mov	x10, x0
	mov 	x11, x1
	mov	x12, x2
	mov	x13, x3

2:

	// Clear BSS
	ldr	x1, =__bss_start
	ldr	w2, =__bss_size
3:
	cbz	w2, 4f		// If BSS SIZE == 0, don't clear it
	str	xzr, [x1], #8	// x1 will +8 bytes after each time
	sub	w2, w2, #1
	cbnz	w2, 3b

4:
	// self clone to 0x30000
	mov	x1, #0x30000
	ldr	x2, =_start
	ldr	w3, =__code_size

5:
	cbz	w3, 6f		// If done, jump to kernel (0x80000)
	ldr	w4, [x2], #4	// each time 4 bytes
	str	w4, [x1], #4
	sub 	w3, w3, #1
	cbnz	w3, 5b
6:
	bl 	relo

	mov 	x1, #0x80000
	str	xzr, [x1]
	bl	read_kernel	//start reading kernel
	mov	x0, x10
	mov 	x1, x11
	mov	x2, x12
	mov	x3, x13

	mov 	x30, #(0x80000)	// Jump to 0x80000
	ret

relo:
	sub	x30, x30, #(0x80000 - 0x30000)
	ret


8:
	b	#0x80000	// Jump to kernel start
	// Never return !!!
1:
	b 	1b 		// Halt

