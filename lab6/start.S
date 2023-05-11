#include "include/mmu.h"
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
	bl	el2_to_el1	// Jump to EL1
	ldr	x1, =0x80000
	mov	sp, x1		// Initialize the sp at _start
				// Note: stack is growing lower

	ldr	x1, =TCR_CONFIG_DEFAULT
	//mov	x1, #0x80100010
	msr	tcr_el1, x1	// Setup TCR_EL1
	ldr	x1, =( \
	(MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
	(MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
	)
	msr	mair_el1, x1	// Setup mair_el1
	mov	x1, 0
	mov	x2, 0x1000
	ldr	x3, =BOOT_PGD_ATTR
	orr	x3, x2, x3
	str	x3, [x1]

PUD:
	ldr	x3, = BOOT_PGD_ATTR // This part need fine-gran
	mov	x4, 0x2000	// The address of PMD(L3)
	orr	x4, x3, x4
	str	x4, [x2]
	ldr	x3, = BOOT_PUD_ATTR // This part of mem is peripherals
	mov	x4, 0x40000000
	orr	x4, x3, x4
	str	x4, [x2, 8]

PMD:	// FOR now only not open L4 yet
	ldr	x6, = BOOT_NORMAL_ATTR	// Normal memory(need to point L4?)
	ldr	x7, = BOOT_DEVICE_ATTR	// Device memory(need align)
	mov	x9, 0x2000	// The PMD
	mov	x10, 512	// entries count
	mov	x11, 0		// Memory pointer start from 0x0
	ldr	x12, =0x3f000000	// The bound of normal memory

	// NOTE: follow llvm loop terminology
L3_set_header:
	cbz	x10, L3_set_exit
	cmp	x11, x12
	blt	L3_nor_exiting		// goto set normal memory

L3_dev_exiting:
	orr	x14, x11, x7		// Set to device attr
	b	L3_set_latch

L3_nor_exiting:
	orr	x14, x11, x6		// Set to normal attr
	b	L3_set_latch

L3_set_latch:
	str	x14, [x9]		// Store the setting
	add	x9, x9, #8		// Each record is 64bits
	add	x11, x11, #0x200000	// memory pointer += 2MB
	sub	x10, x10, #1		// count -= 1
	b	L3_set_header

L3_set_exit:
	// Nothing to do


	msr	ttbr0_el1, x1
	msr	ttbr1_el1, x1
	mrs	x3, sctlr_el1
	orr	x3, x3, 1
	msr	sctlr_el1, x3	// End of virtual mm

	//msr	ttbr0_el1, x1
	//mrs	x3, sctlr_el1
	//orr	x3, x3, 1
	//msr	sctlr_el1, x3

	ldr	x3, =boot_rest
	br	x3


boot_rest:
	// Clear BSS
	ldr	x1, =__bss_start
	ldr	x2, =__bss_size
3:
	cbz	w2, 4f		// If BSS SIZE == 0, don't clear it
	str	xzr, [x1], #8
	sub	x2, x2, #1
	cbnz	x2, 3b

4:
	bl 	main		// Jump to main	
	b 	1b

el2_to_el1:
	mov 	x4, (1 << 31)	// EL1 uses aarch64, Not using x0-3
	msr 	hcr_el2, x4
	mov	x4, 0x3c5	// EL1h with interrupt disable.
	msr	spsr_el2, x4
	msr	elr_el2, lr
	eret



