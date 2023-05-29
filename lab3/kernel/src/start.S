.section ".text.boot"

.global _start

_start: //get cpu id, let cpu0 working only.
	bl from_el2_to_el1
//	bl set_exception_vector_table
	adr x1, exception_vector_table  //vbar_el1: vetor base address reg.
	msr vbar_el1, x1

	mrs	x1, mpidr_el1
	and	x1, x1,#0xFF //check cpu id
	cbz	x1, 2f //if o, jump to 2
		

// enter busy loop
1:
	wfe
	b	1b
	
2: //cpu id ==0
	//set top of stack just before our code (stack grows to a lower address)
	ldr	x1, = _start //ldr:load address
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
	bl	main //find from link
  // for failsafe, halt this core too
	b	1b

from_el2_to_el1:
	mov x1, (1<<31)		//hcr_el2: hypervisor configuration reg. The Execution state for EL1 is AArch64.
	msr hcr_el2, x1		
	mov x1, 0x3c5		//spsr_el2: save process state reg. 0x3c5-> 0b00111100101; 1111:DAIF disable, 0101:EL1h
	msr spsr_el2, x1	
	msr elr_el2, lr 	//elr_el2: return address
	eret				//exception return (el2->el1)

set_exception_vector_table:
	adr x1, exception_vector_table  //vbar_el1: vetor base address reg.
	msr vbar_el1, x1