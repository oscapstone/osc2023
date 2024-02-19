#include "virtual_mem.h"
#include "stdlib.h"

extern uint64_t par_el1;

void setup_kernel_space_mapping()
{
    /* three-level 2MB block mapping */
    uint64_t *PGD = (uint64_t *)0x1000;
    uint64_t *PUD = (uint64_t *)0x2000; // L1 table, entry points to L2 table or 1GB block
    uint64_t *PMD = (uint64_t *)0x3000; // L2 table, entry points to L3 table or 2MB block

    /*
     * Set Identity Paging
     * 0x00000000 ~ 0x3f000000: Normal  // PUD[0]
     * 0x3f000000 ~ 0x40000000: Device  // PUD[0]
     * 0x40000000 ~ 0x80000000: Device  // PUD[1]
     * PGD[0] = (uint64_t)PUD | BOOT_PGD_ATTR
     */

    /* 1st entry points to a L1 table */
    PGD[0] = (uint64_t)PUD | PD_TABLE;

    /* 1st 1GB mapped by L2 table, where L2 table pointed by 1st entry of PUD */
    PUD[0] = (uint64_t)PMD | PD_TABLE;
    /* 2nd 1GB mapped by the 2nd entry of PUD */
    PUD[1] = 0x40000000 | BOOT_PUD_ATTR; // 2nd 1GB mapped by the 2nd entry of PUD

    /*
     * Note for following for:
     *     <<21 because each entry of PMD is 2MB=1<<21
     *     504 = 0x3f000000 / 2MB
     *     512 = 0x40000000 / 2MB
     */

    // 0x00000000 ~ 0x3f000000: Normal
    for (uint64_t i = 0; i < 504; i++)
        PMD[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK;

    // 0x3f000000 ~ 0x40000000: Device
    for (uint64_t i = 504; i < 512; i++)
        PMD[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
}

uint64_t *new_page_table()
{
    uint64_t *table_addr = my_malloc(PAGE_SIZE);
    memset(table_addr, 0, PAGE_SIZE);
    return table_addr;
}

void map_pages(uint64_t *pgd, uint64_t va_start, uint64_t pa_start, int num)
{
    if (pgd == NULL || pa_start == 0)
    {
        printf("[ERROR] map_pages(), pgd=0x%p pa_start=0x%p\r\n", pgd, (void *)pa_start);
        return;
    }

    int index[4]; // index of each table for L0~3
    uint64_t va = 0;
    uint64_t *table = NULL;
    uint64_t entry = 0;
    pa_start = KERNEL_VA_TO_PA(pa_start);
    for (int n = 0; n < num; ++n)
    {
        // Get index of each level
        va = (uint64_t)(va_start + n * PAGE_SIZE);
        va >>= 12;
        index[3] = va & 0x1ff;
        va >>= 9;
        index[2] = va & 0x1ff;
        va >>= 9;
        index[1] = va & 0x1ff;
        va >>= 9;
        index[0] = va & 0x1ff;
        table = (uint64_t *)KERNEL_PA_TO_VA((uint64_t)pgd);
        entry = 0;

        // Find the address of level3 table
        for (int lv = 0; lv < 3; lv++)
        { // map lv0~2

            // Allocate a table that table[index[lv]] can point to
            if (table[index[lv]] == 0)
            {
                table[index[lv]] = PD_ACCESS | KERNEL_VA_TO_PA(new_page_table()) | PD_TABLE;
            }

            // Remove attributes at low 12 bits
            entry = CLEAR_LOW_12bit(table[index[lv]]);

            // Next level
            table = (uint64_t *)KERNEL_PA_TO_VA(entry); // address of the first entry of next level table
        }

        // leve3, aka PTE
        if (table[index[3]] != 0)
            printf("Warning, in map_pages(), PTE[%d]=%lx alread mapped\r\n", index[3], table[index[3]]);
        table[index[3]] = (pa_start + n * PAGE_SIZE) | PD_ACCESS | PD_USER_RW | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PAGE;
    }
}

void *virtual_mem_translate(void *virtual_addr)
{
    asm volatile("mov x4,    %0\n" ::"r"(virtual_addr));
    asm volatile("at  s1e0r, x4\n");
    uint64_t frame_addr = (uint64_t)read_sysreg(par_el1) & 0xFFFFFFFFF000; // physical frame address
    uint64_t pa = frame_addr | ((uint64_t)virtual_addr & 0xFFF);           // combine 12bits offset
    if ((read_sysreg(par_el1) & 0x1) == 1)
        printf("Error, virtual_mem_translate() failed\r\n");
    return (void *)pa;
}