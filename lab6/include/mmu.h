#ifndef MMU_H
#define MMU_H
/*
 * Setup TCR
 */
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0 )| (( 64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)


/*
 * Setup MAIR
 */
#define MAIR_DEVICE_nGnRnE 0b00000000 // bit6: For kernel access
#define MAIR_NORMAL_NOCACHE 0b01000100 // user/kernel can access
#define MAIR_IDX_DEVICE_nGnRnE 0	// Index to MAIR
#define MAIR_IDX_NORMAL_NOCACHE 1

/************************************************************************
 * setup PGD and PUD
 * NOTE: Each page will translate 9-bit of address (512 entries)
 * PUD -> 1 GB, PMD -> 2MB, PTE -> 4KB
 ************************************************************************/
#define PD_TABLE (0b11) // Show that this record point to page table
#define PD_BLOCK (0b01)	// Show that this record point to virtual address
#define PD_ACCESS (1 << 10) // access flag, if not set -> page fault
#define BOOT_PGD_ATTR PD_TABLE // PGD the toppest page table

// The PUD contain the address point to blocks
// NOTE: the attribute place at [12:2] -> need shift
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define BOOT_DEVICE_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)
#define BOOT_NORMAL_ATTR (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK)



#endif // MMU_H
