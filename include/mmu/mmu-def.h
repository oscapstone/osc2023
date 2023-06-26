#ifndef __MMU_DEF_H
#define __MMU_DEF_H
// #include "type.h"

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1
#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE 0b11
#define PD_UNX (1 << 54)
#define PD_PNX (1 << 53)
#define PD_USERACCESS (1 << 6)
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR_DEVICE (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define BOOT_PUD_ATTR_NORMAL (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define BOOT_PUD_TABLE PD_TABLE
#define BOOT_PUD_ATTR BOOT_PUD_ATTR_DEVICE
#define BOOT_PMD_ATTR_NORMAL (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)
#define BOOT_PMD_ATTR_DEVICE (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define PROC_PTE_ATTR_NORMAL (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE)
#define PROC_PTE_ATTR_DEVICE (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE)

#define kernel_va2pa(x) ((unsigned long long)(x) & (~(0xffff000000000000ULL)))
#define kernel_pa2va(x) ((unsigned long long)(x) | (0xffff000000000000ULL))
// #define user_va2pa(x) ((unsigned long long)(x) & (0xffffffffULL))
// #define user_pa2va(x) ((unsigned long long)(x) | (0x0000ffff00000000ULL))

#define PROT_READ (1)
#define PROT_WRITE (2)
#define PROT_EXEC (4)

#define KERNEL_SPACE_OFFSET 0xffff000000000000

#endif