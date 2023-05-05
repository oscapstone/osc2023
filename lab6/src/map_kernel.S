#include "map_kernel.h"

.global set_mmu
.type set_mmu , @function
set_mmu:
	bl	set_tcr_el1
	bl	set_mair_el1
	bl	identity_paging
	ret

set_tcr_el1:
	ldr x0, =TCR_CONFIG_DEFAULT
	msr tcr_el1, x0								//translation control register
	ret

set_mair_el1:
	ldr x0, =((MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)))
	msr mair_el1, x0
	ret

identity_paging:
	mov x0, 0 									// PGD's page frame at 0x0
	mov x1, 0x1000 								// PUD's page frame at 0x1000 (4KB)
	mov x4, 0x2000								// PMD's page frame

	ldr x2, =BOOT_PGD_ATTR
	orr x2, x1, x2 								// entry of PUD which point to a page table
	str x2, [x0]								// first entry of PGD point to PUD

	ldr x2, =BOOT_PUD_ATTR
	mov x3, 0x00000000
	orr x3, x2, x3
	str x3, [x1] 								// 1st 1GB mapped by the 1st entry of PUD
	mov x3, 0x40000000
	orr x3, x2, x3
	str x3, [x1, 8] 							// 2nd 1GB mapped by the 2nd entry of PUD

RAM_START:
	mov x5, 0x3F000000							// RAM's end
	mov x6, 0x0									// RAM's start
	mov x7, #0									// PMD's entry index
repeat1:
	sub x8, x5, x6								// x8 = x5-x6
	cbz x8, RAM_DONE							// 2MB page_table alloc done
	ldr x2, =RAM_PMD_ATTR
	orr x2, x2, x6								// set x6's address on 2MB page_table
	str x2, [x4, x7]							// entry of this page point to PMD
	add x6, x6, 0x20000							// calculate next page_table's start address
	add x7, x7, #8
	b	repeat1

RAM_DONE:
MMIO_START:
	mov x5, 0x7E000000							//MMIO's end
	mov x6, 0x3F000000							//MMIO's start
repeat2:
	sub x8, x5, x6
	cbz x8, MMIO_DONE
	ldr x2, =RAM_PMD_ATTR
	orr x2, x2, x6
	str x2, [x4, x7]
	add x6, x6, 0x20000
	add x7, x7, #8
	b   repeat2

MMIO_DONE:
	msr ttbr0_el1, x0 							// load PGD to the bottom translation-based register
	msr ttbr1_el1, x0							// also load PGD to the upper translation based register
	mrs x2, sctlr_el1
	orr x2, x2, 1
	msr sctlr_el1, x2 							// enable MMU, cache remains disabled
	
	ldr x2, =boot_rest							// indirect branch to the virtual address
	br	x2

.global context_switch
.type context_switch , @function
context_switch:
	dsb ish										//ensure write has completed
	msr ttbr0_el1,x0							//switch translation based address
	tlbi vmalle1is								//invalidate all TLB entries
	dsb ish										//ensure completion of TLB invalidation
	isb											//clear pipeline
	ret
