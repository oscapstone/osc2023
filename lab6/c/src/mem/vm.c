#include "oscos/mem/vm.h"

#include <stdint.h>

#include "oscos/console.h"
#include "oscos/libc/string.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/shared-page.h"
#include "oscos/sched.h"
#include "oscos/utils/align.h"
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

static int _vm_cmp_mem_regions_by_start(const mem_region_t *const r1,
                                        const mem_region_t *const r2,
                                        void *const _arg) {
  (void)_arg;

  if (r1->start < r2->start)
    return -1;
  if (r1->start > r2->start)
    return 1;
  return 0;
}

static int _vm_cmp_va_and_mem_region(const void *const va,
                                     const mem_region_t *const region,
                                     void *const _arg) {
  (void)_arg;

  if (va < region->start)
    return -1;
  if (va > region->start)
    return 1;
  return 0;
}

void vm_mem_regions_insert_region(mem_regions_t *const regions,
                                  const mem_region_t *const region) {
  rb_insert(
      &regions->root, sizeof(mem_region_t), region,
      (int (*)(const void *, const void *, void *))_vm_cmp_mem_regions_by_start,
      NULL);
}

const mem_region_t *
vm_mem_regions_find_region(const mem_regions_t *const regions, void *const va) {
  const mem_region_t *const region = rb_predecessor(
      regions->root, va,
      (int (*)(const void *, const void *, void *))_vm_cmp_va_and_mem_region,
      NULL);

  return region && va < (void *)((char *)region->start + region->len) ? region
                                                                      : NULL;
}

static page_table_entry_t *_vm_new_pgd(void) {
  const spage_id_t new_pgd_page_id = shared_page_alloc();
  if (new_pgd_page_id < 0)
    return NULL;

  page_table_entry_t *const result =
      pa_to_kernel_va(page_id_to_pa(new_pgd_page_id));

  memset(result, 0, 1 << PAGE_ORDER);

  return result;
}

vm_addr_space_t vm_new_addr_space(void) {
  return (vm_addr_space_t){.mem_regions = {.root = NULL}, .pgd = _vm_new_pgd()};
}

static void _vm_clone_pgd(page_table_entry_t *const pgd) {
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

vm_addr_space_t vm_clone_addr_space(const vm_addr_space_t addr_space) {
  rb_node_t *const new_regions_root =
      rb_clone(addr_space.mem_regions.root, sizeof(mem_region_t), NULL, NULL);
  if (!new_regions_root)
    return (vm_addr_space_t){.mem_regions = (mem_regions_t){.root = NULL}};

  _vm_clone_pgd(addr_space.pgd);
  return (vm_addr_space_t){.mem_regions =
                               (mem_regions_t){.root = new_regions_root},
                           .pgd = addr_space.pgd};
}

static void _vm_drop_page_table(page_table_entry_t *const page_table,
                                const size_t level) {
  const page_id_t page_table_page_id =
      pa_to_page_id(kernel_va_to_pa(page_table));

  if (shared_page_getref(page_table_page_id) == 1) { // About to be freed.
    for (size_t i = 0; i < 512; i++) {
      if (page_table[i].b0) {
        const pa_t next_level_pa = page_table[i].addr << PAGE_ORDER;
        if (level == 0) {
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

static void _vm_drop_pgd(page_table_entry_t *const pgd) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _vm_drop_page_table(pgd, 3);

  CRITICAL_SECTION_LEAVE(daif_val);
}

void vm_drop_addr_space(const vm_addr_space_t addr_space) {
  _vm_drop_pgd(addr_space.pgd);
  rb_drop(addr_space.mem_regions.root, NULL);
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

  // Set attributes.

  const bool is_accessible =
      mem_region->prot & (PROT_READ | PROT_WRITE | PROT_EXEC);
  const bool is_writable = mem_region->prot & PROT_WRITE;
  const bool is_executable = mem_region->prot & PROT_EXEC;

  const unsigned ap = ((unsigned)!is_writable << 1) | (unsigned)is_accessible;

  pte_entry->b0 = 1;
  pte_entry->b1 = 1;
  const union {
    block_page_descriptor_lower_t s;
    unsigned u;
  } lower = {.s = (block_page_descriptor_lower_t){
                 .attr_indx = 0x1, .ap = ap, .af = 1}};
  pte_entry->lower = lower.u;
  const union {
    block_page_descriptor_upper_t s;
    unsigned u;
  } upper = {.s = (block_page_descriptor_upper_t){.pxn = !is_executable,
                                                  .uxn = !is_executable}};
  pte_entry->upper = upper.u;

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

  const mem_region_t *const region =
      vm_mem_regions_find_region(&addr_space->mem_regions, va);
  if (!region)
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

  // Set attributes.

  const bool is_accessible =
      mem_region->prot & (PROT_READ | PROT_WRITE | PROT_EXEC);
  const bool is_writable = mem_region->prot & PROT_WRITE;
  const bool is_executable = mem_region->prot & PROT_EXEC;

  const unsigned ap = ((unsigned)!is_writable << 1) | (unsigned)is_accessible;

  const union {
    block_page_descriptor_lower_t s;
    unsigned u;
  } lower = {.s = (block_page_descriptor_lower_t){
                 .attr_indx = 0x1, .ap = ap, .af = 1}};
  pte_entry->lower = lower.u;
  const union {
    block_page_descriptor_upper_t s;
    unsigned u;
  } upper = {.s = (block_page_descriptor_upper_t){.pxn = !is_executable,
                                                  .uxn = !is_executable}};
  pte_entry->upper = upper.u;

  return true;
}

static vm_map_page_result_t _vm_cow(vm_addr_space_t *const addr_space,
                                    void *const va) {
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

  const mem_region_t *const region =
      vm_mem_regions_find_region(&addr_space->mem_regions, va);
  if (!_cow_page(region, pte_entry)) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return VM_MAP_PAGE_NOMEM;
  }

  // Set ttbr0_el1 again, as the PGD may have changed.
  vm_switch_to_addr_space(addr_space);

  CRITICAL_SECTION_LEAVE(daif_val);

  return VM_MAP_PAGE_SUCCESS;
}

vm_map_page_result_t
vm_handle_permission_fault(vm_addr_space_t *const addr_space, void *const va,
                           const int access_mode) {
  const mem_region_t *const region =
      vm_mem_regions_find_region(&addr_space->mem_regions, va);
  if (access_mode & region->prot) {
    return _vm_cow(addr_space, va);
  } else { // Illegal access.
#ifdef VM_ENABLE_DEBUG_LOG
    if (access_mode & PROT_EXEC) {
      console_puts(
          "DEBUG: vm: Attempted to execute from a non-executable page");
    } else if (access_mode & PROT_WRITE) {
      console_puts("DEBUG: vm: Attempted to write to a non-writable page");
    } else {
      console_puts("DEBUG: vm: Attempted to access a non-accessible page");
    }
#endif
    return VM_MAP_PAGE_SEGV;
  }
}

static page_table_entry_t *
_vm_remove_region_from_pgd_rec(page_table_entry_t *const page_table,
                               void *const start_va, void *const end_va,
                               size_t level, void *const block_start,
                               page_table_entry_t *const prev_level_entry) {
  void *const block_end = (char *)block_start + (1 << (9 * level + PAGE_ORDER));

  if (start_va == block_start && end_va == block_end) {
    _vm_drop_page_table(page_table, level);
    if (prev_level_entry) {
      prev_level_entry->b0 = false;
    }
    return NULL;
  } else {
    page_table_entry_t *const new_page_table =
        level == 0 ? _vm_clone_unshare_pte(page_table)
                   : _vm_clone_unshare_page_table(page_table);
    if (!new_page_table)
      return NULL;

    if (prev_level_entry) {
      prev_level_entry->addr = kernel_va_to_pa(new_page_table) >> PAGE_ORDER;

      // Since the page table is not shared, we can remove the read-only bit
      // now.

      union {
        table_descriptor_upper_t s;
        unsigned u;
      } upper = {.u = prev_level_entry->upper};
      upper.s.aptable = 0x0;
      prev_level_entry->upper = upper.u;
    }

    size_t subblock_stride = 1 << (9 * (level - 1) + PAGE_ORDER);

    for (size_t i = 0; i < 512; i++) {
      if (new_page_table[i].b0) {
        void *const subblock_start = (char *)block_start + i * subblock_stride,
                    *const subblock_end =
                        (char *)subblock_start + subblock_stride;

        void *const max_start =
                        start_va > subblock_start ? start_va : subblock_start,
                    *const min_end =
                        end_va < subblock_end ? end_va : subblock_end;

        if (max_start < min_end) {
          if (level == 0) {
            const page_id_t page_id =
                pa_to_page_id(new_page_table[i].addr << PAGE_ORDER);
            shared_page_decref(page_id);
          } else {
            page_table_entry_t *const next_level_page_table =
                pa_to_kernel_va(new_page_table[i].addr << PAGE_ORDER);
            _vm_remove_region_from_pgd_rec(next_level_page_table, max_start,
                                           min_end, level - 1, subblock_start,
                                           &new_page_table[i]);
          }
        }
      }
    }

    return new_page_table;
  }
}

static page_table_entry_t *
_vm_remove_region_from_pgd(page_table_entry_t *const pgd, void *const start_va,
                           void *const end_va) {
  return _vm_remove_region_from_pgd_rec(pgd, start_va, end_va, 3, (void *)0,
                                        NULL);
}

bool vm_remove_region(vm_addr_space_t *const addr_space, void *const start_va) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const mem_region_t *const mem_region = rb_search(
      addr_space->mem_regions.root, start_va,
      (int (*)(const void *, const void *, void *))_vm_cmp_va_and_mem_region,
      NULL);
  void *const end_va = (char *)start_va + mem_region->len;

  rb_delete(
      &addr_space->mem_regions.root, start_va,
      (int (*)(const void *, const void *, void *))_vm_cmp_va_and_mem_region,
      NULL);

  page_table_entry_t *const new_pgd =
      _vm_remove_region_from_pgd(addr_space->pgd, start_va, end_va);
  // Note: `_vm_remove_region_from_pgd` returns NULL when the last region is
  // removed, i.e., a new page table need not be allocated. However, in all
  // places where `vm_remove_region` is used, the region to be removed is never
  // the last one. Therefore, checking for an out-of-memory condition in this
  // way is fine.
  if (!new_pgd) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return false;
  }

  addr_space->pgd = new_pgd;

  CRITICAL_SECTION_LEAVE(daif_val);
  return true;
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

static void *_vm_find_mmap_addr(const vm_addr_space_t addr_space,
                                const size_t len) {
  void *addr = (void *)(1 << PAGE_ORDER);
  const mem_region_t *const first_predecessor = rb_predecessor(
      addr_space.mem_regions.root, addr,
      (int (*)(const void *, const void *, void *))_vm_cmp_va_and_mem_region,
      NULL);
  if (first_predecessor &&
      addr < (void *)((char *)first_predecessor->start +
                      first_predecessor
                          ->len)) { // The initial address (0x1000) is taken.
    addr = (char *)first_predecessor->start + first_predecessor->len;
  }

  while (addr < (void *)(0x1000000000000ULL - len)) {
    const mem_region_t *const begin_successor = rb_successor(
        addr_space.mem_regions.root, addr,
        (int (*)(const void *, const void *, void *))_vm_cmp_va_and_mem_region,
        NULL);
    if (!begin_successor ||
        (void *)((char *)addr + len) <= begin_successor->start) {
      // Found a large enough space.
      return addr;
    }
    addr = (char *)begin_successor->start + begin_successor->len;
  }

  return NULL;
}

void *vm_decide_mmap_addr(const vm_addr_space_t addr_space, void *const va,
                          const size_t len) {
  const size_t effective_len = ALIGN(len, 1 << PAGE_ORDER);

  // Check for exact fit.

  const bool va_is_page_aligned =
      ((uintptr_t)va & ((1 << PAGE_ORDER) - 1)) == 0;
  if (va && va_is_page_aligned) {
    const mem_region_t *const begin_predecessor = rb_predecessor(
        addr_space.mem_regions.root, va,
        (int (*)(const void *, const void *, void *))_vm_cmp_va_and_mem_region,
        NULL);
    if (!begin_predecessor ||
        (void *)((char *)begin_predecessor->start + begin_predecessor->len) <=
            va) { // The predecessor doesn't cover the start address.
      const mem_region_t *const end_predecessor = rb_predecessor(
          addr_space.mem_regions.root, (char *)va + effective_len - 1,
          (int (*)(const void *, const void *,
                   void *))_vm_cmp_va_and_mem_region,
          NULL);
      if (end_predecessor ==
          begin_predecessor) { // There is nothing between the start and end
                               // addresses.
        // Exact fit is possible.
        return va;
      }
    }
  }

  return _vm_find_mmap_addr(addr_space, effective_len);
}
