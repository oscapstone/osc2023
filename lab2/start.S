.section ".text.boot"

.global _start

_start:
	mov 	x27, x0			// DTB base
        
        ldr     x1, =__bss_start	// clear bss
        ldr     w2, =__bss_size
bss:      
        str     xzr, [x1], #8
        sub     w2, w2, #1
        cbnz    w2, bss
		
	mov 	x1, #0x60000	//relocate
	ldr	x2, =__loader_size
	ldr 	x3, =__bootloader_start
relocate:
	ldr	x4, [x3], #8
	str 	x4, [x1], #8
	sub  	x2, x2, #1
	cbnz	x2, relocate

	mov	sp, #0x60000	// set top of stack just before our code (stack grows to a lower address per AAPCS64)
	bl	main-0x20000
