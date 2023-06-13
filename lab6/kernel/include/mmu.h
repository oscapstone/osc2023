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

#define PD_TABLE                0b11L
#define PD_BLOCK                0b01L
#define PD_UNX                  (1L << 54)  // user no-execute
#define PD_KNX                  (1L << 53)  
#define PD_ACCESS               (1L << 10)
#define PD_RDONLY               (1L << 7)   // read-only
#define PD_UK_ACCESS            (1L << 6)   // user accessible

#define PERIPHERAL_START        0x3c000000L
#define PERIPHERAL_END          0x3f000000L
#define USER_KERNEL_BASE        0x00000000L
#define USER_STACK_BASE         0xfffffffff000L
#define USER_SIGNAL_WRAPPER_VA  0xffffffff9000L

#define MMU_PGD_BASE            0x1000L
#define MMU_PGD_ADDR            (MMU_PGD_BASE + 0x0000L)
#define MMU_PUD_ADDR            (MMU_PGD_BASE + 0x1000L)
#define MMU_PTE_ADDR            (MMU_PGD_BASE + 0x2000L)

#define BOOT_PGD_ATTR           (PD_TABLE)
#define BOOT_PUD_ATTR           (PD_TABLE | PD_ACCESS)
#define BOOT_PTE_ATTR_nGnRnE    (PD_BLOCK | PD_ACCESS | (MAIR_DEVICE_nGnRnE << 2) | PD_UNX | PD_KNX | PD_UK_ACCESS)
#define BOOT_PTE_ATTR_NOCACHE   (PD_BLOCK | PD_ACCESS | (MAIR_NORMAL_NOCACHE << 2))

// DDI0487A_a_armv8_arm.pdf-p.1901
#define MEMFAIL_DATA_ABORT_LOWER 0b100100
#define MEMFAIL_INST_ABORT_LOWER 0b100000

// Translation fault p.1525
#define TF_LEVEL0 0b000100
#define TF_LEVEL1 0b000101
#define TF_LEVEL2 0b000110
#define TF_LEVEL3 0b000111

#ifndef __ASSEMBLER__

#include "sched.h"

// DDI0487A_a_armv8_arm.pdf-p.1512
typedef struct{
    unsigned int iss : 25, // Instruction specific 
                 il : 1,   // Instruction length bit 32bit(0) or 64bits(1)
                 ec : 6;   // Exception class
} esr_el1_t;

typedef struct vm_area_struct
{
    list_head_t listhead;
    unsigned long virt_addr;
    unsigned long phys_addr;
    unsigned long area_size;
    unsigned long rwx;   // 1, 2, 4
    int is_alloced;

} vm_area_struct_t;

void *set_2M_kernel_mmu(void *x0);
void map_one_page(size_t *pgd_p, size_t va, size_t pa, size_t flag);

void mmu_add_vma(struct thread *t, size_t va, size_t size, size_t pa, size_t rwx, int is_alloced);
void mmu_del_vma(struct thread *t);
void mmu_map_pages(size_t *pgd_p, size_t va, size_t size, size_t pa, size_t flag);
void mmu_free_page_tables(size_t *page_table, int level);

void mmu_memfail_abort_handle(esr_el1_t* esr_el1);

#endif //__ASSEMBLER__

#endif /* _MMU_H_ */
