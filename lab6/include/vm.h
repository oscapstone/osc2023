#ifndef VM_H
#define VM_H
#include "mmu.h"
#include <stddef.h>
#include <stdint.h>

//===================================================================
// Useful functions
//===================================================================
uint64_t phy2vir(void *);
uint64_t vir2phy(void *);

int map_vm(uint64_t *, uint64_t vm, uint64_t pm, int length, int prop);
int copy_vm(uint64_t *, uint64_t *);
uint64_t walk_vm(uint64_t *, uint64_t vm);

// For Demand paging
typedef struct VM_Node {
  uint64_t phy;
  uint64_t vir;
  int prop;
  struct VM_Node *next;
  struct VM_Node *prev;
} vm_node;

// List operation for list
int vm_list_add(vm_node **, uint64_t vir, uint64_t phy, int prop);
uint64_t vm_list_delete(vm_node **, uint64_t vir);
int vm_list_copy(vm_node *from, vm_node **to);
int vm_list_dump(vm_node *);

// MMAP FLAGS
#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_ANONYMOUS 0x0020
#define MAP_POPULATE 0x4000

#endif // VM_H
