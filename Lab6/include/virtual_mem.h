#ifndef _VIRTUAL_MEM_H
#define _VIRTUAL_MEM_H

#include <stdint.h>

#define PAGE_SIZE 0x1000
#define USTACK_VA 0xffffffffb000
#define STACK_SIZE 0x4000
#define UPROG_VA 0x0

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_ACCESS (1 << 10) // The access flag, a page fault is generated if not set.
#define PD_USER_RW (0b1 << 6)
#define PD_USER_R (0b11 << 6)
#define PD_UXN (1L << 54)
#define PD_PXN (1L << 53)

#define DEFAULT_THREAD_VA_CODE_START 0x0000
#define DEFAULT_THREAD_VA_STACK_START 0xFFFFFFFFB000

#define ENTRY_GET_ATTRS(num) ((num)&0xFFFF000000000FFF)
#define CLEAR_LOW_12bit(num) ((num)&0xFFFFFFFFFFFFF000)
#define KERNEL_VA_TO_PA(addr) (((uint64_t)(addr)) & 0x0000FFFFFFFFFFFFLL)
#define KERNEL_PA_TO_VA(addr) (((uint64_t)(addr)) | 0xFFFF000000000000LL)

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) | (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)

void setup_kernel_space_mapping();
uint64_t *new_page_table();
void map_pages(uint64_t *pgd, uint64_t va_start, uint64_t pa_start, int num);
void *virtual_mem_translate(void *virtual_addr);

#endif /*_VIRTUAL_MEM_H */