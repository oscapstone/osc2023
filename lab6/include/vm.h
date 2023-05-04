#ifndef VM_H
#define VM_H
#include "mmu.h"
#include <stdint.h>

//===================================================================
//Useful functions
//===================================================================
uint64_t phy2vir(void*);
uint64_t vir2phy(void*);

int map_vm(uint64_t*, uint64_t vm, uint64_t pm, int length);

#endif // VM_H
