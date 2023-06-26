
#ifndef __MMU_H
#define __MMU_H
#include "mmu/mmu-def.h"
#include "type.h"

uint64_t mappages(void *pgd, uint64_t va, uint64_t size, uint64_t pa);
#endif