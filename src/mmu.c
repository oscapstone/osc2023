#include "mmu.h"
#include "mm.h"
#include "exception.h"
#include "utils.h"

void mmu_setup()
{
    write_sysreg(tcr_el1, TCR_EL1_VALUE);
    write_sysreg(mair_el1, 
        (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
        (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    );

}

unsigned long get_pte(unsigned long va)
{
    disable_interrupt();

    asm("at s1e0r, %0\n\t" ::"r"(va));
    unsigned long pte = read_sysreg(par_el1);

    test_enable_interrupt();
    return pte;
}

unsigned long el0_va2pa(unsigned long va)
{
    unsigned long entry = get_pte(va);
    if (entry & 1)
    {
        uart_write_string("Failed map virtual addr\n");
    }
    unsigned long offset = va & 0xfff;
    return (unsigned long)(PA2VA((entry & 0xfffffffff000L) | offset));
}

pte_t pool[2][PAGE_SIZE];
void kspace_map()
{
    /*  three-level 2MB block mapping    */

    pte_t *p0 = pool[0];//alloc_pages(1);

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
    pte_t *p1 = pool[1];//alloc_pages(1);
    for (int i = 0; i < 512; i++)
    {
        p1[i] = 0x40000000 | (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
    }

    asm volatile("dsb ish\n\t");

    IDENTITY_TT_L1_VA[0] = (pte_t)VA2PA(p0) | PD_TABLE;
    IDENTITY_TT_L1_VA[1] = (pte_t)VA2PA(p1) | PD_TABLE;
}

void *pgtable_walk(unsigned long *table, unsigned long idx) {
    if (!table) {
        uart_write_string("pgtable_walk failed");
        return 0;
    }
    if (!table[idx]) {
        void *new_page = alloc_pages(1);
        if (!new_page) {
            uart_write_string("pgtable_walk cannot get a new page by alloc_pages(1)!\n");
            return 0;
        }
        memset(new_page, 0, PAGE_SIZE);
        table[idx] = VA2PA((unsigned long)new_page) | PD_TABLE;
    }
    return (void*)PA2VA(table[idx] & PAGE_MASK);
}

void *pgtable_walk_pte(unsigned long *table, unsigned long idx, unsigned long paddr) {
    if (!table) {
        uart_write_string("pgtable_walk_pte failed");
        return 0;
    }
    if (!table[idx]) {
        if (paddr == 0) {
            void *new_page = alloc_pages(1);
            if (!new_page)
                return 0;
            memset(new_page, 0, PAGE_SIZE);
            table[idx] = VA2PA((unsigned long)new_page) | PTE_NORMAL_ATTR | PD_USER_RW;
        } else
            table[idx] = paddr | PTE_NORMAL_ATTR | PD_USER_RW;
    }
    return (void*)PA2VA(table[idx] & PAGE_MASK);
}

void pgtable_walk_block(unsigned long *table, unsigned long idx, unsigned long paddr) {
    if (!table) {
        uart_write_string("pgtable_walk_block failed");
        return 0;
    }
    if (!table[idx]) {
        table[idx] = paddr | PUD1_ATTR | PD_USER_RW;
    }
}

void *walk(unsigned long *pgd, unsigned long vaddr, unsigned long paddr) {
    void *pud;
    void *pmd;
    void *pte;
    
    pud = pgtable_walk(pgd, pgd_index(vaddr));
    pmd = pgtable_walk(pud, pud_index(vaddr));
    pte = pgtable_walk(pmd, pmd_index(vaddr));
    return pgtable_walk_pte(pte, pte_index(vaddr), paddr) + (vaddr & (PAGE_SIZE-1));
}

void *mappages(unsigned long *pgd, unsigned long vaddr, unsigned long size, unsigned long paddr) {
    if (!pgd)
        return 0;
    for (int i=0 ; i<size ; i+=PAGE_SIZE) {
        if (!paddr)
            walk(pgd, vaddr+i, 0);
        else
            walk(pgd, vaddr+i, paddr+i);
    }
    return (void*)vaddr;
}

void identity_paging(unsigned long *pgd, unsigned long vaddr, unsigned long paddr) {
    if (pgd)
        return 0;
    void *pud;
    pud = pgtable_walk(pgd, pgd_index(vaddr));
    pgtable_walk_block(pud, pud_index(vaddr), paddr);
}

void create_pgd(unsigned long **pgd) {
    void *new_page = alloc_pages(1);
    if (!new_page)
        return;
    memset(new_page, 0, PAGE_SIZE);
    *pgd = new_page;
}

void free_pgtable(unsigned long *table, int level) {
    void *next;
    for(int i=0 ; i<512 ; i++) {
        if (table[i]) {
            next = PA2VA(table[i] & PAGE_MASK);
            if (level != 4) 
                free_pgtable(next, level+1);
            // if device memory
            if (level == 4 && VA2PA(next) >= 0x3C000000)
                continue;
            free_page(next);
        }
    }
}

void free_pgd(unsigned long *pgd) {
    void *pud;
    for(int i=0 ; i<512 ; i++) {
        if (pgd[i]) {  
            pud = PA2VA(pgd[i] & PAGE_MASK);
            free_pgtable(pud, 2);
            free_page(pud);
        }
    }
    free_page(pgd);
}

/*
        level
    pgd   1
    pud   2
    pmd   3
    pte   4
*/
void fork_pgtable(unsigned long *ptable, unsigned long *ctable, int level) {
    void *pnext;
    void *cnext;

    for(int i=0 ; i<512 ; i++) {
        if (ptable[i]) {
            pnext = PA2VA(ptable[i] & PAGE_MASK);
            if (level == 4) {
                cnext = pgtable_walk_pte(ctable, i, 0);
                //check if cnext is peripheral
                if (cnext >= 0x3C000000 && cnext < 0x3F000000)
                    continue;
                memcpy(cnext, pnext, PAGE_SIZE);
            } else {
                cnext = pgtable_walk(ctable, i);
                fork_pgtable(pnext, cnext, level+1);
            }
        }
    }
}

void fork_pgd(unsigned long *ppgd, unsigned long *cpgd) {
    void *ppud;
    void *cpud;
    
    if (!ppgd || !cpgd) 
        return;
    
    for(int i=0 ; i<512 ; i++) {
        if (ppgd[i]) {  
            ppud = PA2VA(ppgd[i] & PAGE_MASK);
            cpud = pgtable_walk(cpgd, i);
            fork_pgtable(ppud, cpud, 2);
        }
    }
}
