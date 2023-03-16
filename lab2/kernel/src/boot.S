.section ".text.boot"

.global _start

_start: //get cpu id, let cpu0 working only.
	mrs	x0, mpidr_el1
	and	x0, x0,#0xFF //check cpu id
	cbz	x0, 2f //if o, jump to 2
		

// enter busy loop
1:
	wfe
	b	1b
	
2: //cpu id ==0
	//set top of stack just before our code (stack grows to a lower address)
	ldr	x0, = _start //ldr:load address
	mov 	sp, x0	 //sp:stack pointer
   //clear bss (check if empty)
	ldr	x0, =__bss_start
	ldr	x1, =__bss_size
	
3:
	cbz	x1, 4f //check if bss is empty =0, jump to 4
	str	xzr, [x0], #8 //str: store to register
	sub	x1, x1, #1
	cbnz	x1, 3b 


4:// jump to C code
	bl	main //find from link
  // for failsafe, halt this core too
	b	1b
