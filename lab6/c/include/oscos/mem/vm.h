#ifndef OSCOS_MEM_VM_H
#define OSCOS_MEM_VM_H

#include "oscos/mem/types.h"
#include "oscos/mem/vm/page-table.h"

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
} mem_region_t;

typedef struct {
  mem_region_t text_region, stack_region, vc_region;
  page_table_entry_t *pgd;
} vm_addr_space_t;

typedef enum {
  VM_MAP_PAGE_SUCCESS,
  VM_MAP_PAGE_SEGV,
  VM_MAP_PAGE_NOMEM
} vm_map_page_result_t;

page_table_entry_t *vm_new_pgd(void);
void vm_clone_pgd(page_table_entry_t *pgd);
void vm_drop_pgd(page_table_entry_t *pgd);
vm_map_page_result_t vm_map_page(vm_addr_space_t *addr_space, void *va);
vm_map_page_result_t vm_cow(vm_addr_space_t *addr_space, void *va);
void vm_switch_to_addr_space(const vm_addr_space_t *addr_space);

#endif
