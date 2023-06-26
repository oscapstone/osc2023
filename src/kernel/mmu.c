#include "mmu/mmu.h"
#include "type.h"
#include "mem/mem.h"


uint64_t* find_pte(void *pgd, unsigned long long va, uint8_t create) {
    pgd = kernel_pa2va(pgd);

    uint64_t *PGD = pgd;

    uint64_t pud_idx = (va >> 39) & 0b111111111;
    if((PGD[pud_idx] & 1) == 0) {
        if(!create) return 0;
        uint64_t new_PUD = (uint64_t)cmalloc((1 << 12));
        PGD[pud_idx] = (uint64_t)kernel_va2pa((new_PUD | PD_TABLE));
    }
    uint64_t *PUD = (uint64_t*)kernel_pa2va((PGD[pud_idx] >> 12) << 12);
    uint64_t pmd_idx = (va >> 30) & 0b111111111;
    if((PUD[pmd_idx] & 1) == 0) {
        if(!create) return 0;
        uint64_t new_PMD = (uint64_t)cmalloc(1 << 12);
        PUD[pmd_idx] = (uint64_t)kernel_va2pa(new_PMD | PD_TABLE);
    }
    uint64_t *PMD = (uint64_t*)kernel_pa2va((PUD[pmd_idx] >> 12) << 12);
    uint64_t pte_idx = (va >> 21) & 0b111111111;
    if((PMD[pte_idx] & 1) == 0) {
        if(!create) return 0;
        uint64_t new_PTE = (uint64_t)cmalloc(1 << 12);
        PMD[pte_idx] = (uint64_t)kernel_va2pa(new_PTE | PD_TABLE);
    }

    uint64_t *PTE = (uint64_t*)kernel_pa2va((PMD[pte_idx] >> 12) << 12);
    uint64_t pe_idx = (va >> 12) & 0b111111111;

    return &PTE[pe_idx];
}

uint64_t mappages(void *pgd, uint64_t va, uint64_t size, uint64_t pa) {

    for(uint64_t i = 0; i < size; i += (1 << 12)) {
        uint64_t *pte_entry = find_pte(pgd, va + i, 1);

        uint64_t page_attr = PROC_PTE_ATTR_NORMAL;
        page_attr |= PD_USERACCESS;
        *pte_entry = ((pa + i) | page_attr);
    }
}
int mmu_map_peripheral(uint64_t ttbr0_el1) {
    for(uint64_t va = 0x30000000; va < 0x40010000; va += (1 << 12)) {
        uint64_t *PTE = find_pte(ttbr0_el1, va, 1);
        uint64_t newPTE = (va | PROC_PTE_ATTR_DEVICE | PD_USERACCESS);
        *PTE = newPTE;
    }
}