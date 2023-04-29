#ifndef _MMU_H_
#define _MMU_H_

#include "stddef.h"

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB          ((0b00 << 14) | (0b10 << 30))
#define TCR_CONFIG_DEFAULT      (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE      0b00000000
#define MAIR_NORMAL_NOCACHE     0b01000100
#define MAIR_IDX_DEVICE_nGnRnE  0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define PERIPHERAL_START        0x3c000000L
#define PERIPHERAL_END          0x3f000000L
#define USER_KERNEL_BASE        0x00000000L
#define THREAD_STACK_SIZE       0x4000L
#define USER_STACK_BASE         0xfffffffff000L - THREAD_STACK_SIZE

#define PD_TABLE     0b11L
#define PD_BLOCK     0b01L
#define PD_PAGE      0b11L
#define PD_NE_EL0    (1L << 54)
#define PD_NE_EL1    (1L << 53)
#define PD_ACCESS    (1L << 10)
#define PD_UK_ACCESS (1L << 6)
#define PD_RDONLY    (1L << 7)

#define BOOT_PGD_ATTR (PD_TABLE)
#define BOOT_PUD_ATTR (PD_TABLE)
#define BOOT_PMD_ATTR (PD_TABLE)
#define BOOT_PTE_DEVICE_nGnRnE_ATTR \
  (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE | PD_UK_ACCESS | PD_NE_EL0 | PD_NE_EL1)
#define BOOT_PTE_NORMAL_NOCACHE_ATTR \
  (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)


#define kernel_mmu_addr         0x1000L
#define kernel_pgd_addr         (kernel_mmu_addr + 0x0000)
#define kernel_pud_addr         (kernel_mmu_addr + 0x1000)
#define kernel_pmd_addr         (kernel_mmu_addr + 0x2000)
#define kernel_pte_addr         (kernel_mmu_addr + 0x4000)

#ifndef __ASSEMBLER__
void* set_2M_kernel_mmu(void* x0);
//void* set_vm_4tables(void* x0);
void init_page_table(unsigned long **table, int level);
void map_one_page(size_t *pgd_p, size_t va, size_t pa);
void mappages(size_t *pgd_p, size_t va, size_t size, size_t pa);
void free_page_tables(size_t *page_table, int level);
#endif //__ASSEMBLER__

#endif /* _MMU_H_ */
