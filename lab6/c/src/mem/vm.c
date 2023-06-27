#include "oscos/mem/vm.h"

#include <stdint.h>

#include "oscos/libc/string.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/shared-page.h"
#include "oscos/sched.h"
#include "oscos/utils/critical-section.h"

// Symbol defined in the linker script.
extern char _kernel_vm_base[];

pa_t kernel_va_to_pa(const void *const va) {
  return (pa_t)((uintptr_t)va - (uintptr_t)_kernel_vm_base);
}

void *pa_to_kernel_va(const pa_t pa) {
  return (void *)((uintptr_t)pa + (uintptr_t)_kernel_vm_base);
}

pa_range_t kernel_va_range_to_pa_range(const va_range_t range) {
  return (pa_range_t){.start = kernel_va_to_pa(range.start),
                      .end = kernel_va_to_pa(range.end)};
}

page_table_entry_t *vm_new_pgd(void) {
  const spage_id_t new_pgd_page_id = shared_page_alloc();
  if (new_pgd_page_id < 0)
    return NULL;

  page_table_entry_t *const result =
      pa_to_kernel_va(page_id_to_pa(new_pgd_page_id));

  memset(result, 0, 1 << PAGE_ORDER);

  return result;
}

void vm_clone_pgd(page_table_entry_t *const pgd) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const page_id_t pgd_page_id = pa_to_page_id(kernel_va_to_pa(pgd));
  shared_page_incref(pgd_page_id);

  for (size_t i = 0; i < 512; i++) {
    if (pgd[i].b0) {
      union {
        table_descriptor_upper_t s;
        unsigned u;
      } upper = {.u = pgd[i].upper};
      upper.s.aptable = 0x2;
      pgd[i].upper = upper.u;
    }
  }

  CRITICAL_SECTION_LEAVE(daif_val);
}

static void _vm_drop_page_table(page_table_entry_t *const page_table,
                                const size_t level) {
  const page_id_t page_table_page_id =
      pa_to_page_id(kernel_va_to_pa(page_table));

  if (shared_page_getref(page_table_page_id) == 1) { // About to be freed.
    for (size_t i = 0; i < 512; i++) {
      if (page_table[i].b0) {
        const pa_t next_level_pa = page_table[i].addr << PAGE_ORDER;
        if (level == 1) {
          shared_page_decref(pa_to_page_id(next_level_pa));
        } else {
          page_table_entry_t *const next_level_page_table =
              pa_to_kernel_va(next_level_pa);
          _vm_drop_page_table(next_level_page_table, level - 1);
        }
      }
    }
  }

  shared_page_decref(page_table_page_id);
}

void vm_drop_pgd(page_table_entry_t *const pgd) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _vm_drop_page_table(pgd, 3);

  CRITICAL_SECTION_LEAVE(daif_val);
}

static page_table_entry_t *
_vm_clone_unshare_page_table(page_table_entry_t *const page_table) {
  const page_id_t page_table_page_id =
      pa_to_page_id(kernel_va_to_pa(page_table));

  if (shared_page_getref(page_table_page_id) == 1) // No need to clone.
    return page_table;

  shared_page_decref(page_table_page_id);

  const spage_id_t new_page_table_page_id = shared_page_alloc();
  if (new_page_table_page_id < 0)
    return NULL;
  page_table_entry_t *const new_page_table =
      pa_to_kernel_va(page_id_to_pa(new_page_table_page_id));

  for (size_t i = 0; i < 512; i++) {
    if (page_table[i].b0) {
      const pa_t next_level_pa = page_table[i].addr << PAGE_ORDER;
      const page_id_t next_level_page_id = pa_to_page_id(next_level_pa);

      shared_page_incref(next_level_page_id);

      union {
        table_descriptor_upper_t s;
        unsigned u;
      } upper = {.u = page_table[i].upper};
      upper.s.aptable = 0x2;
      page_table[i].upper = upper.u;
    }
  }

  memcpy(new_page_table, page_table, 1 << PAGE_ORDER);

  return new_page_table;
}

static page_table_entry_t *
_vm_clone_unshare_pte(page_table_entry_t *const pte) {
  const page_id_t pte_page_id = pa_to_page_id(kernel_va_to_pa(pte));

  if (shared_page_getref(pte_page_id) == 1) // No need to clone.
    return pte;

  shared_page_decref(pte_page_id);

  const spage_id_t new_pte_page_id = shared_page_alloc();
  if (new_pte_page_id < 0)
    return NULL;
  page_table_entry_t *const new_pte =
      pa_to_kernel_va(page_id_to_pa(new_pte_page_id));

  for (size_t i = 0; i < 512; i++) {
    if (pte[i].b0) {
      const pa_t next_level_pa = pte[i].addr << PAGE_ORDER;
      const page_id_t next_level_page_id = pa_to_page_id(next_level_pa);

      shared_page_incref(next_level_page_id);

      union {
        block_page_descriptor_lower_t s;
        unsigned u;
      } lower = {.u = pte[i].lower};
      lower.s.ap = 0x3;
      pte[i].lower = lower.u;
    }
  }

  memcpy(new_pte, pte, 1 << PAGE_ORDER);

  return new_pte;
}

static void _init_backed_page(const mem_region_t *const mem_region,
                              void *const va, void *const kernel_va) {
  void *const va_page_start =
      (void *)((uintptr_t)va & ~((1 << PAGE_ORDER) - 1));
  size_t offset = (char *)va_page_start - (char *)mem_region->start;

  const size_t copy_len =
      offset > mem_region->backing_storage_len ? 0
      : mem_region->backing_storage_len - offset > 1 << PAGE_ORDER
          ? 1 << PAGE_ORDER
          : mem_region->backing_storage_len - offset;
  memcpy(kernel_va, (char *)mem_region->backing_storage_start + offset,
         copy_len);
  memset((char *)kernel_va + copy_len, 0, (1 << PAGE_ORDER) - copy_len);
}

static bool _map_page(const mem_region_t *const mem_region, void *const va,
                      page_table_entry_t *const pte_entry) {
  switch (mem_region->type) {
  case MEM_REGION_BACKED: {
    const spage_id_t page_id = shared_page_alloc();
    if (page_id < 0) {
      return false;
    }

    const pa_t page_pa = page_id_to_pa(page_id);
    pte_entry->addr = page_pa >> PAGE_ORDER;

    _init_backed_page(mem_region, va, pa_to_kernel_va(page_pa));

    break;
  }

  case MEM_REGION_LINEAR: {
    const size_t offset = (uintptr_t)va - (uintptr_t)mem_region->start;
    pte_entry->addr =
        ((uintptr_t)mem_region->backing_storage_start + offset) >> PAGE_ORDER;
    break;
  }

  default:
    __builtin_unreachable();
  }

  pte_entry->b0 = 1;
  pte_entry->b1 = 1;
  const union {
    block_page_descriptor_lower_t s;
    unsigned u;
  } lower = {.s = (block_page_descriptor_lower_t){.ap = 0x1, .af = 1}};
  pte_entry->lower = lower.u;

  return true;
}

static page_table_entry_t *
_vm_clone_unshare_pte_entry(vm_addr_space_t *const addr_space, void *const va) {
  page_table_entry_t *page_table =
      _vm_clone_unshare_page_table(addr_space->pgd);
  if (!page_table)
    return NULL;
  addr_space->pgd = page_table;
  page_table_entry_t *prev_level_entry = &page_table[(uintptr_t)va >> 39];

  for (size_t level = 2; level > 0; level--) {
    if (prev_level_entry->b0) {
      page_table = _vm_clone_unshare_page_table(
          pa_to_kernel_va(prev_level_entry->addr << PAGE_ORDER));
      if (!page_table)
        return NULL;
    } else {
      const spage_id_t page_table_page_id = shared_page_alloc();
      if (page_table_page_id < 0)
        return NULL;
      page_table = pa_to_kernel_va(page_id_to_pa(page_table_page_id));
      memset(page_table, 0, 1 << PAGE_ORDER);
      prev_level_entry->b0 = 1;
      prev_level_entry->b1 = 1;
    }

    // Since the page table is not shared, we can remove the read-only bit now.

    union {
      table_descriptor_upper_t s;
      unsigned u;
    } upper = {.u = prev_level_entry->upper};
    upper.s.aptable = 0x0;
    prev_level_entry->upper = upper.u;

    const pa_t page_table_pa = kernel_va_to_pa(page_table);
    prev_level_entry->addr = page_table_pa >> 12;
    prev_level_entry =
        &page_table[((uintptr_t)va >> (12 + level * 9)) & ((1 << 9) - 1)];
  }

  if (prev_level_entry->b0) {
    page_table = _vm_clone_unshare_pte(
        pa_to_kernel_va(prev_level_entry->addr << PAGE_ORDER));
    if (!page_table)
      return NULL;
  } else {
    const spage_id_t page_table_page_id = shared_page_alloc();
    if (page_table_page_id < 0)
      return NULL;
    page_table = pa_to_kernel_va(page_id_to_pa(page_table_page_id));
    memset(page_table, 0, 1 << PAGE_ORDER);
    prev_level_entry->b0 = 1;
    prev_level_entry->b1 = 1;
  }

  // Since the page table is not shared, we can remove the read-only bit now.

  {
    union {
      table_descriptor_upper_t s;
      unsigned u;
    } upper = {.u = prev_level_entry->upper};
    upper.s.aptable = 0x0;
    prev_level_entry->upper = upper.u;
  }

  const pa_t page_table_pa = kernel_va_to_pa(page_table);
  prev_level_entry->addr = page_table_pa >> 12;
  prev_level_entry = &page_table[((uintptr_t)va >> 12) & ((1 << 9) - 1)];

  return prev_level_entry;
}

vm_map_page_result_t vm_map_page(vm_addr_space_t *const addr_space,
                                 void *const va) {
  // Check the validity of the VA.

  if (!((addr_space->text_region.start <= va &&
         va < (void *)((char *)addr_space->text_region.start +
                       addr_space->text_region.len)) ||
        (addr_space->stack_region.start <= va &&
         va < (void *)((char *)addr_space->stack_region.start +
                       addr_space->stack_region.len)) ||
        (addr_space->vc_region.start <= va &&
         va < (void *)((char *)addr_space->vc_region.start +
                       addr_space->vc_region.len))))
    return VM_MAP_PAGE_SEGV;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  // Walk the page table.

  page_table_entry_t *const pte_entry =
      _vm_clone_unshare_pte_entry(addr_space, va);
  if (!pte_entry) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return VM_MAP_PAGE_NOMEM;
  }

  // Map the page.

  mem_region_t *const region =
      addr_space->text_region.start <= va &&
              va < (void *)((char *)addr_space->text_region.start +
                            addr_space->text_region.len)
          ? &addr_space->text_region
      : addr_space->stack_region.start <= va &&
              va < (void *)((char *)addr_space->stack_region.start +
                            addr_space->stack_region.len)
          ? &addr_space->stack_region
      : addr_space->vc_region.start <= va &&
              va < (void *)((char *)addr_space->vc_region.start +
                            addr_space->vc_region.len)
          ? &addr_space->vc_region
          : (__builtin_unreachable(), NULL);
  if (!_map_page(region, va, pte_entry)) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return VM_MAP_PAGE_NOMEM;
  }

  // Set ttbr0_el1 again, as the PGD may have changed.
  vm_switch_to_addr_space(addr_space);

  CRITICAL_SECTION_LEAVE(daif_val);

  return VM_MAP_PAGE_SUCCESS;
}

static bool _cow_page(const mem_region_t *const mem_region,
                      page_table_entry_t *const pte_entry) {
  switch (mem_region->type) {
  case MEM_REGION_BACKED: {
    const page_id_t src_page_id = pa_to_page_id(pte_entry->addr << PAGE_ORDER);
    const spage_id_t page_id = shared_page_clone_unshare(src_page_id);
    if (page_id < 0)
      return false;

    const pa_t page_pa = page_id_to_pa(page_id);
    pte_entry->addr = page_pa >> PAGE_ORDER;

    break;
  }

  case MEM_REGION_LINEAR: {
    // No-op.
    break;
  }

  default:
    __builtin_unreachable();
  }

  const union {
    block_page_descriptor_lower_t s;
    unsigned u;
  } lower = {.s = (block_page_descriptor_lower_t){.ap = 0x1, .af = 1}};
  pte_entry->lower = lower.u;

  return true;
}

vm_map_page_result_t vm_cow(vm_addr_space_t *const addr_space, void *const va) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  // Walk the page table.

  page_table_entry_t *const pte_entry =
      _vm_clone_unshare_pte_entry(addr_space, va);
  if (!pte_entry) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return VM_MAP_PAGE_NOMEM;
  }

  // Map the page.

  mem_region_t *const region =
      addr_space->text_region.start <= va &&
              va < (void *)((char *)addr_space->text_region.start +
                            addr_space->text_region.len)
          ? &addr_space->text_region
      : addr_space->stack_region.start <= va &&
              va < (void *)((char *)addr_space->stack_region.start +
                            addr_space->stack_region.len)
          ? &addr_space->stack_region
      : addr_space->vc_region.start <= va &&
              va < (void *)((char *)addr_space->vc_region.start +
                            addr_space->vc_region.len)
          ? &addr_space->vc_region
          : (__builtin_unreachable(), NULL);
  if (!_cow_page(region, pte_entry)) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return VM_MAP_PAGE_NOMEM;
  }

  // Set ttbr0_el1 again, as the PGD may have changed.
  vm_switch_to_addr_space(addr_space);

  CRITICAL_SECTION_LEAVE(daif_val);

  return VM_MAP_PAGE_SUCCESS;
}

void vm_switch_to_addr_space(const vm_addr_space_t *const addr_space) {
  const pa_t pgd_pa = kernel_va_to_pa(addr_space->pgd);
  __asm__ __volatile__(
      "dsb ish           // Ensure writes have completed.\n"
      "msr ttbr0_el1, %0 // Switch page table.\n"
      "tlbi vmalle1is    // Invalidate all TLB entries.\n"
      "dsb ish           // Ensure completion of TLB invalidatation.\n"
      "isb               // Clear pipeline."
      :
      : "r"((uint64_t)pgd_pa)
      : "memory");
}

pa_t vm_translate_addr(const page_table_entry_t *const pgd, void *const va) {
  const page_table_entry_t *page_table = pgd;
  for (size_t level = 3; level > 0; level--) {
    page_table = pa_to_kernel_va(
        page_table[((uintptr_t)va >> (12 + 9 * level)) & ((1 << 9) - 1)].addr
        << PAGE_ORDER);
  }
  return page_table[((uintptr_t)va >> 12) & ((1 << 9) - 1)].addr << PAGE_ORDER |
         ((uintptr_t)va & ((1 << 12) - 1));
}
