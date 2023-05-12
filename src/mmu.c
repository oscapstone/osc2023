#include "mmu.h"

void kspace_map()
{
    /*  three-level 2MB block mapping    */

    pte_t *p0 = alloc_pages(1);

    /*  0x00000000 ~ 0x3F000000 for normal mem  */
    for (int i = 0; i < 504; i++)
    {
        p0[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK;
    }
    /*  0x3F000000 ~ 0x40000000 for device mem  */
    for (int i = 504; i < 512; i++)
    {
        p0[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
    }

    /*  0x40000000 ~ 0x80000000 for device mem  */
    pte_t *p1 = alloc_pages(1);
    for (int i = 0; i < 512; i++)
    {
        p1[i] = 0x40000000 | (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
    }

    asm volatile("dsb ish\n\t");

    IDENTITY_TT_L1_VA[0] = (pte_t)VA2PA(p0) | PD_TABLE;
    IDENTITY_TT_L1_VA[1] = (pte_t)VA2PA(p1) | PD_TABLE;
}