#ifndef _MMU_H
#define _MMU_H

#define VA_START            0xffff000000000000
#define VA_MASK             0x0000ffffffffffff


/* If upper 16 bits are all equal to 0 then PGD address stored in ttbr0_el1 is used, 
and if the address starts with 0xffff(first 16 bit are all equal to 1) then PGD address stored in the ttbr1_el1 is selected. 
The architecture also ensures that a process running at EL0 can never access virtual addresses started with 0xffff without generating a synchronous exception.
*/


#define PAGE_MASK			      0xfffffffffffff000

/*Translation Control Register (TCR)*/
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB          ((0b00 << 14) | (0b10 << 30))
#define TCR_CONFIG_DEFAULT      (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE      0b00000000
#define MAIR_NORMAL_NOCACHE     0b01000100
#define MAIR_IDX_DEVICE_nGnRnE  0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define MAIR_VALUE \
( \
  (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
  (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
)

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PT_ENTRY 0b11
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)	

#define PTE_USR_RO_PTE      (PD_ACCESS | (0b11<<6) | (MAIR_IDX_NORMAL_NOCACHE<<2) | PT_ENTRY)
#define PTE_USR_RW_PTE      (PD_ACCESS | (0b01<<6) | (MAIR_IDX_NORMAL_NOCACHE<<2) | PT_ENTRY)

#define SCTLR_MMU_DISABLED  0
#define SCTLR_MMU_ENABLED   1

#define PGD_SHIFT   (12 + 3*9)
#define PUD_SHIFT   (12 + 2*9)
#define PMD_SHIFT   (12 + 9)

#ifndef __ASSEMBLER__

#include "sched.h"

void map_page(struct task_struct *task, unsigned long va, unsigned long page);
unsigned long map_table(unsigned long *table, unsigned long shift, unsigned long va, int* new_table);
void map_table_entry(unsigned long *pte, unsigned long va, unsigned long pa);
unsigned long va2phys(unsigned long va);

#endif

#endif