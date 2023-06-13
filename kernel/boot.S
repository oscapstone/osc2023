#include "mm.h"
#include "mmu.h"
//put .text.boot as the first symbol in text section
.section ".text.boot"
//.global makes the symbol visible to ld.
#define PA(p) (p - 0xffff000000000000)
.global _start
_start:
	//move dtb address to x5
	mov x5, x0
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
	bl from_el2_to_el1
	// setup tcr
    ldr     x0, =TCR_EL1_VALUE
    msr     tcr_el1, x0

	// MAIR_EL1, Memory Attribute Indirection Register
	ldr x0, =( \
  	(MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
  	(MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
	)
	msr mair_el1, x0

	//identitiy paging
    
	mov x0, 0x1000 // PGD's page frame at 0x0
    mov x1, 0x1000 // page size
    bl my_bzero
	mov x0, 0x2000 // PUD's page frame at 0x1000
	mov x1, 0x1000
    bl my_bzero

    mov x0, 0x1000
    mov x1, 0x2000

		//setup pgd
	ldr x2, = BOOT_PGD_ATTR
	orr x2, x1, x2 // combine the physical address of next level page with attribute.
	str x2, [x0]

		//setup pud
	ldr x2, = BOOT_PUD_ATTR
	mov x3, 0x00000000
	orr x3, x2, x3
	str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD
	mov x3, 0x40000000
	orr x3, x2, x3
	str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD

	msr ttbr0_el1, x0 // load PGD to the bottom translation-based register.
	msr ttbr1_el1, x0 // also load PGD to the upper translation based register.

	// enable MMU
	mrs x2, sctlr_el1
	orr x2 , x2, 1
	msr sctlr_el1, x2 // enable MMU, cache remains disabled

	// indirect branch to the virtual address
	ldr x2, = boot_rest
	br x2

boot_rest:
	bl set_exception_vector_table

	adr	x0, __bss_start
	adr	x1, __bss_end
	sub	x1, x1, x0
	bl 	my_bzero

	adr    x2, _dtb_ptr
    str    x5, [x2]
    
    // change sp to virtual address
    ldr x1, =KERN_BASE
	add x1, x1, #LOW_MEMORY
	//add sp, sp, x1
	mov sp, x1

	mov x0, fp
    orr fp, x0, x1

	//ldr x9, =main
	//br x9
	bl	main        // branch PC to main function which is defined in main.c
	b 	proc_hang		// should never come here
    
.global _dtb_ptr
.section .data
_dtb_ptr: .dc.a 0x0