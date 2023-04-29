#include "bcm2837/rpi_mmu.h"
#include "mmu.h"
#include "memory.h"
#include "u_string.h"
// PGD's page frame at 0x1000
// PUD's page frame at 0x2000
// two-level translation (1G) -> three-level translation (2MB)
void* set_2M_kernel_mmu(void* x0)
{
    //unsigned long *pud_table = kmalloc(0x1000);
    unsigned long *pud_table = (unsigned long *)0x3000;
    for (int i = 0; i < 512; i++)
    {
        // 0x3F000000 to 0x3FFFFFFF for peripherals
        if ((0x00000000 + (0x200000L) * i) >= 0x3F000000L)
        {
            pud_table[i] = PD_ACCESS + PD_BLOCK + (0x00000000 + (0x200000L) * i) + (MAIR_DEVICE_nGnRnE << 2) + PD_UK_ACCESS;
        }else
        {
            pud_table[i] = PD_ACCESS + PD_BLOCK + (0x00000000 + (0x200000L) * i) + (MAIR_IDX_NORMAL_NOCACHE << 2);
        }
    }

    //unsigned long *pud_table2 = kmalloc(0x1000);
    unsigned long *pud_table2 = (unsigned long *)0x4000;
    for (int i = 0; i < 512; i++)
    {
        pud_table2[i] = PD_ACCESS + PD_BLOCK + (0x40000000L + (0x200000L) * i) + (MAIR_IDX_NORMAL_NOCACHE << 2);
    }

    // set PUD
    *(unsigned long *)(kernel_pud_addr) = PD_ACCESS + (MAIR_IDX_NORMAL_NOCACHE << 2) + PD_TABLE + (unsigned long)pud_table;
    *(unsigned long *)(kernel_pud_addr + 0x8) = PD_ACCESS + (MAIR_IDX_NORMAL_NOCACHE << 2) + PD_TABLE + (unsigned long)pud_table2;

    return x0;
}

// pa,va aligned to 4K
void map_one_page(size_t *virt_pgd_p, size_t va, size_t pa)
{
    size_t *table_p = virt_pgd_p;
    for (int level = 0; level < 4; level++)
    {
        unsigned int idx = (va >> (39 - level * 9)) & 0x1ff;

        if (level == 3)
        {
            table_p[idx] = pa;
            table_p[idx] |= PD_ACCESS | PD_TABLE | (MAIR_IDX_NORMAL_NOCACHE << 2) | (PD_UK_ACCESS);
            return;
        }

        if(!table_p[idx])
        {
            size_t* newtable_p =kmalloc(0x1000);
            memset(newtable_p, 0, 0x1000);
            table_p[idx] = VIRT_TO_PHYS((size_t)newtable_p);
            table_p[idx] |= PD_ACCESS | PD_TABLE | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_UK_ACCESS;
        }

        table_p = (size_t*)PHYS_TO_VIRT((size_t)(table_p[idx] & ENTRY_ADDR_MASK));
    }
}

//give 
void mappages(size_t *virt_pgd_p, size_t va, size_t size, size_t pa)
{
    for (size_t s = 0; s < size; s+=0x1000)
    {
        map_one_page(virt_pgd_p, va + s, pa + s);
    }
}

void free_page_tables(size_t *page_table, int level)
{
    size_t *table_virt = (size_t*)PHYS_TO_VIRT((char*)page_table);
    for (int i = 0; i < 512; i++)
    {
        if (table_virt[i] != 0)
        {
            size_t *next_table = (size_t*)(table_virt[i] & ENTRY_ADDR_MASK);
            if (table_virt[i] & PD_TABLE)
            { 
                if(level!=2)free_page_tables(next_table, level + 1);
                table_virt[i] = 0L;
                kfree(PHYS_TO_VIRT((char *)next_table));
            }
        }
    }
}
