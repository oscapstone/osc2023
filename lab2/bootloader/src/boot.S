.section ".text.boot"

.global _start

_start: //get cpu id, let cpu0 working only.
	mrs	x1, mpidr_el1
	and	x1, x1,#0xFF //check cpu id
	cbz	x1, 2f //if o, jump to 2
		

// enter busy loop
1:
	wfe
	b	1b
	
2: //cpu id ==0
	//set top of stack just before our code (stack grows to a lower address)
	//ldr	x1, = _start //ldr:load address
	ldr	x1, =__stack_top
	mov 	sp, x1	 //sp:stack pointer
   //clear bss (check if empty)
	ldr	x1, =__bss_start
	ldr	w2, =__bss_size
	
3:
	cbz	w2, 4f //check if bss is empty =0, jump to 4
	str	xzr, [x1], #8 //str: store to register
	sub	w2, w2, #1
	cbnz	w2, 3b 


4:// jump to C code
	//bl	main //find from link
	bl	relocate
  // for failsafe, halt this core too
	b	1b
