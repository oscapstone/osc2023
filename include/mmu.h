#ifndef _MMU_H
#define _MMU_H
#include "mmio.h"
#ifndef __ASSEMBLER__
#include "stdint.h"
typedef unsigned long long pte_t;
extern void kspace_map();
void create_pgd(unsigned long **pgd);
void free_pgd(unsigned long *pgd);

void *walk(unsigned long *pgd, unsigned long vaddr, unsigned long paddr);
void *mappages(unsigned long *pgd, unsigned long vaddr, unsigned long size, unsigned long paddr);

void identity_paging(unsigned long *pgd, unsigned long vaddr, unsigned long paddr);
#endif
// TCR_EL1, Translation Control Register
#define TTBR0_EL1_REGION_BIT    48
#define TTBR1_EL1_REGION_BIT    48
#define TTBR0_EL1_GRANULE       0b00 // 4KB
#define TTBR1_EL1_GRANULE       0b10 // 4KB

#define TCR_EL1_T0SZ            ((64 - TTBR0_EL1_REGION_BIT) << 0)
#define TCR_EL1_T1SZ            ((64 - TTBR1_EL1_REGION_BIT) << 16)
#define TCR_EL1_TG0             (TTBR0_EL1_GRANULE << 14)
#define TCR_EL1_TG1             (TTBR1_EL1_GRANULE << 30)

#define TCR_EL1_VALUE           (TCR_EL1_T0SZ | TCR_EL1_T1SZ | TCR_EL1_TG0 | TCR_EL1_TG1)

// MAIR
//nGnRnE (no gather, no re-order, no early write acknowledgment) 
/*
    MAIR
    Device memory                0b0000dd00
    Normal memory                0booooiiii, (oooo != 0000 and iiii != 0000)
        oooo == 0b0100           Outer Non-cacheable (L2)
        iiii == 0b0100           Inner Non-cacheable (L1)
*/
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

// #define PA2VA(pa) (uint64_t)((char *)(pa) + KERN_BASE)
// #define VA2PA(va) (uint64_t)((char *)(va) - KERN_BASE)

#define PA2VA(pa) (uint64_t)((uint64_t)(pa) | KERN_BASE)
#define VA2PA(va) (uint64_t)((uint64_t)(va) & ~KERN_BASE)

// Identity Paging
#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE  0b11 
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR PD_TABLE
//Bits[1:0]
//  Specify the next level is a block/page, page table, or invalid.
//Bits[4:2]
//  The index to MAIR.
//Bits[6]
//  0 for only kernel access, 1 for user/kernel access.
//Bits[7]
//  0 for read-write, 1 for read-only.
//Bits[10]
//  The access flag, a page fault is generated if not set.
//Bits[47:n]:
//  The physical address the entry point to.
//  Note that the address should be aligned to 2^n Byte.
//Bits[53]
//  The privileged execute-never bit, non-executable page frame for EL1 if set.
//Bits[54]
//  The unprivileged execute-never bit, non-executable page frame for EL0 if set.
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define IDENTITY_TT_L0 ((pte_t *)0x1000L)
#define IDENTITY_TT_L1 ((pte_t *)0x2000L)
#define IDENTITY_TT_L0_VA ((pte_t *)PA2VA(IDENTITY_TT_L0))
#define IDENTITY_TT_L1_VA ((pte_t *)PA2VA(IDENTITY_TT_L1))

#define PD_USER_RW      (0b01 << 6)

#define PD_ACCESS       (1 << 10)

#define PGD0_ATTR               PD_TABLE
#define PUD0_ATTR               PD_TABLE
#define PUD1_ATTR               (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define PMD0_ATTR               PD_TABLE
#define PTE_DEVICE_ATTR         (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE)
#define PTE_NORMAL_ATTR         (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define PTE_NORMAL_LAZY_ATTR    ((MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)


#define PGD_SHIFT               39
#define PUD_SHIFT               30
#define PMD_SHIFT               21

#define PTRS_PER_PGD            512
#define PTRS_PER_PUD            512
#define PTRS_PER_PMD            512
#define PTRS_PER_PTE            512

#define pgd_index(vaddr)      (((vaddr) >> PGD_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_offset(mm, vaddr) ((mm)->pgd + pgd_index(vaddr))

#define pud_index(vaddr)      (((vaddr) >> PUD_SHIFT) & (PTRS_PER_PUD - 1))

#define pmd_index(vaddr)      (((vaddr) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))

#define pte_index(vaddr)      (((vaddr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#endif