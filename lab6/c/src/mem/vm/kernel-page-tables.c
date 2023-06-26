#include "oscos/mem/vm/kernel-page-tables.h"

#include <stdalign.h>
#include <stdint.h>

#include "oscos/drivers/board.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/types.h"
#include "oscos/mem/vm.h"
#include "oscos/mem/vm/page-table.h"

alignas(4096) page_table_t kernel_pud = {{.b0 = 1,
                                          .b1 = 0,
                                          .lower = 1 << 8, // AF = 1.
                                          .addr = 0x0,
                                          .upper = 0},
                                         {.b0 = 1,
                                          .b1 = 0,
                                          .lower = 1 << 8, // AF = 1.
                                          .addr = 0x40000,
                                          .upper = 0}};

alignas(4096) page_table_t kernel_pgd = {
    {.b0 = 1,
     .b1 = 1,
     .lower = 0,
     .addr = 0, // The concrete value will be filled by `start.S`.
     .upper = 0}};

static void _map_region_as_rec(const page_id_range_t range,
                               const block_page_descriptor_lower_t lower_attr,
                               const block_page_descriptor_upper_t upper_attr,
                               const size_t level, const page_id_t block_start,
                               page_table_entry_t *const entry) {
  const union {
    block_page_descriptor_lower_t s;
    unsigned u;
  } lower = {.s = lower_attr};
  const union {
    block_page_descriptor_upper_t s;
    unsigned u;
  } upper = {.s = upper_attr};

  const page_id_t block_end = block_start + (1 << (9 * level));

  if (range.start == block_start && range.end == block_end) {
    *entry = (page_table_entry_t){.b0 = 1,
                                  .b1 = level == 0,
                                  .lower = lower.u,
                                  .addr = block_start,
                                  .upper = upper.u};
  } else {
    if (level == 0)
      __builtin_unreachable();

    const page_id_t subblock_stride = 1 << (9 * (level - 1));

    page_table_entry_t *page_table;
    if (entry->b1) { // Table.
      page_table = (page_table_entry_t *)pa_to_kernel_va(entry->addr << 12);
    } else { // Block.
      const pa_t page_table_pa = page_id_to_pa(alloc_pages(0));
      page_table = pa_to_kernel_va(page_table_pa);

      for (size_t i = 0; i < 512; i++) {
        page_table[i] =
            (page_table_entry_t){.b0 = 1,
                                 .b1 = level == 1,
                                 .lower = entry->lower,
                                 .addr = entry->addr + i * subblock_stride,
                                 .upper = entry->upper};
      }
      *entry = (page_table_entry_t){.b0 = 1,
                                    .b1 = 1,
                                    .lower = entry->lower,
                                    .addr = page_table_pa >> 12,
                                    .upper = entry->upper};
    }

    for (size_t i = 0; i < 512; i++) {
      const page_id_t subblock_start = block_start + i * subblock_stride,
                      subblock_end = subblock_start + subblock_stride;

      const page_id_t max_start = range.start > subblock_start ? range.start
                                                               : subblock_start,
                      min_end =
                          range.end < subblock_end ? range.end : subblock_end;

      if (max_start < min_end) {
        _map_region_as_rec(
            (page_id_range_t){.start = max_start, .end = min_end}, lower_attr,
            upper_attr, level - 1, subblock_start, &page_table[i]);
      }
    }
  }
}

static void _map_region_as(const page_id_range_t range,
                           const block_page_descriptor_lower_t lower_attr,
                           const block_page_descriptor_upper_t upper_attr) {
  _map_region_as_rec(range, lower_attr, upper_attr, 3, 0, kernel_pgd);
}

void vm_setup_finer_granularity_linear_mapping(void) {
  // Map RAM as normal memory.
  _map_region_as(
      (page_id_range_t){.start = 0x0,
                        .end = kernel_va_to_pa(PERIPHERAL_BASE) >> 12},
      (block_page_descriptor_lower_t){
          .attr_indx = 1, .ap = 0x0, .sh = 0x0, .af = 1, .ng = 0},
      (block_page_descriptor_upper_t){.contiguous = 0, .pxn = 0, .uxn = 0});
}
