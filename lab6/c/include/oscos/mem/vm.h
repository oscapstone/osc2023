#ifndef OSCOS_MEM_VM_H
#define OSCOS_MEM_VM_H

#include "oscos/mem/types.h"
#include "oscos/mem/vm/page-table.h"
#include "oscos/uapi/sys/mman.h"
#include "oscos/utils/rb.h"

/// \brief Converts a kernel space virtual address into its corresponding
///        physical address.
pa_t kernel_va_to_pa(const void *va) __attribute__((const));

/// \brief Converts a physical address into its corresponding kernel space
///        virtual address.
void *pa_to_kernel_va(pa_t pa) __attribute__((const));

/// \brief Converts a kernel space virtual address range into its corresponding
///        physical address range.
pa_range_t kernel_va_range_to_pa_range(va_range_t range) __attribute__((const));

typedef enum { MEM_REGION_BACKED, MEM_REGION_LINEAR } mem_region_type_t;

typedef struct {
  void *start;
  size_t len;
  mem_region_type_t type;
  const void *backing_storage_start;
  size_t backing_storage_len;
  int prot;
} mem_region_t;

typedef struct {
  rb_node_t *root;
} mem_regions_t;

typedef struct {
  mem_regions_t mem_regions;
  page_table_entry_t *pgd;
} vm_addr_space_t;

typedef enum {
  VM_MAP_PAGE_SUCCESS,
  VM_MAP_PAGE_SEGV,
  VM_MAP_PAGE_NOMEM
} vm_map_page_result_t;

void vm_mem_regions_insert_region(mem_regions_t *regions,
                                  const mem_region_t *region);
const mem_region_t *vm_mem_regions_find_region(const mem_regions_t *regions,
                                               void *va);

vm_addr_space_t vm_new_addr_space(void);
vm_addr_space_t vm_clone_addr_space(vm_addr_space_t addr_space);
void vm_drop_addr_space(vm_addr_space_t pgd);
vm_map_page_result_t vm_map_page(vm_addr_space_t *addr_space, void *va);
vm_map_page_result_t vm_handle_permission_fault(vm_addr_space_t *addr_space,
                                                void *va, int access_mode);
bool vm_remove_region(vm_addr_space_t *addr_space, void *start_va);
void vm_switch_to_addr_space(const vm_addr_space_t *addr_space);

void *vm_decide_mmap_addr(vm_addr_space_t addr_space, void *va, size_t len);

#endif
