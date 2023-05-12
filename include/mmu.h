#ifndef _MMU_H
#define _MMU_H
#include "mmio.h"
#ifndef __ASSEMBLER__
typedef unsigned long long pte_t;
extern void kspace_map();
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
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define PA2VA(pa) ((char *)(pa) + KERN_BASE)
#define VA2PA(va) ((char *)(va) - KERN_BASE)

// Identity Paging
#define PD_TABLE 0b11
#define PD_BLOCK 0b01
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
#define IDENTITY_TT_L0 ((pte_t *)0x0000L)
#define IDENTITY_TT_L1 ((pte_t *)0x1000L)
#define IDENTITY_TT_L0_VA ((pte_t *)PA2VA(0x0000L))
#define IDENTITY_TT_L1_VA ((pte_t *)PA2VA(0x1000L))


#endif