#include "oscos/mem/memmap.h"

#include "oscos/console.h"
#include "oscos/devicetree.h"
#include "oscos/initrd.h"
#include "oscos/libc/stdlib.h"
#include "oscos/panic.h"

#define MAX_N_MEMS 1
#define MAX_N_RESERVED_MEMS 8

// Symbols defined in the linker script.
extern char _skernel[], _ekernel[], _sstack[], _estack[];

static pa_range_t _mems[MAX_N_MEMS];
static size_t _n_mems = 0;
static reserved_mem_entry_t _reserved_mems[MAX_N_RESERVED_MEMS];
static size_t _n_reserved_mems = 0;

static int _mem_range_cmp_by_start(const pa_range_t *const r1,
                                   const pa_range_t *const r2) {
  if (r1->start < r2->start)
    return -1;
  if (r1->start > r2->start)
    return 1;
  return 0;
}

static int
_reserved_mem_entry_cmp_by_start(const reserved_mem_entry_t *const r1,
                                 const reserved_mem_entry_t *const r2) {
  return _mem_range_cmp_by_start(&r1->range, &r2->range);
}

void memmap_init(void) {
  // No-op.
}

bool mem_add(const pa_range_t mem) {
  if (_n_mems == MAX_N_MEMS)
    return false;

  _mems[_n_mems++] = mem;
  return true;
}

void mem_sort(void) {
  qsort(_mems, _n_mems, sizeof(pa_range_t),
        (int (*)(const void *, const void *))_mem_range_cmp_by_start);
}

size_t mem_get_n(void) { return _n_mems; }

const pa_range_t *mem_get(void) { return _mems; }

bool reserved_mem_add(const reserved_mem_entry_t reserved_mem) {
  if (_n_reserved_mems == MAX_N_RESERVED_MEMS)
    return false;

  _reserved_mems[_n_reserved_mems++] = reserved_mem;
  return true;
}

void reserved_mem_sort(void) {
  qsort(_reserved_mems, _n_reserved_mems, sizeof(reserved_mem_entry_t),
        (int (*)(const void *, const void *))_reserved_mem_entry_cmp_by_start);
}

size_t reserved_mem_get_n(void) { return _n_reserved_mems; }

const reserved_mem_entry_t *reserved_mem_get(void) { return _reserved_mems; }

#define ADD_MEM_OR_PANIC(ENTRY)                                                \
  do {                                                                         \
    if (!mem_add(ENTRY))                                                       \
      PANIC("memmap: Ran out of space for memory entries");                    \
  } while (0)

#define RESERVE_OR_PANIC(ENTRY)                                                \
  do {                                                                         \
    if (!reserved_mem_add(ENTRY))                                              \
      PANIC("memmap: Ran out of space for reserved memory entries");           \
  } while (0)

typedef struct {
  bool is_memory_done, is_reserved_memory_done;
  fdt_n_address_size_cells_t root_n_cells, reserved_memory_n_cells;
  const fdt_item_t *reserved_memory_node;
} scan_mem_fdt_traverse_callback_arg_t;

static control_flow_t _scan_mem_fdt_traverse_callback(
    scan_mem_fdt_traverse_callback_arg_t *arg, const fdt_item_t *const node,
    const fdt_traverse_parent_list_node_t *const parent) {
  if (!parent) { // Current node is the root node.
    arg->root_n_cells = fdt_get_n_address_size_cells(node);
  } else if (parent && !parent->parent &&
             strncmp(FDT_NODE_NAME(node), "memory@", 7) ==
                 0) { // Current node is /memory@...
    FDT_FOR_ITEM(node, item) {
      if (FDT_TOKEN(item) == FDT_PROP) {
        const fdt_prop_t *const prop = (const fdt_prop_t *)item->payload;
        if (strcmp(FDT_PROP_NAME(prop), "reg") == 0) {
          const fdt_read_reg_result_t read_result =
              fdt_read_reg(prop, arg->root_n_cells);
          const uint32_t start = read_result.value.address,
                         end = start + read_result.value.size;
          if (read_result.address_overflow || read_result.size_overflow ||
              read_result.value.address > UINT32_MAX ||
              read_result.value.size > UINT32_MAX || end < start)
            PANIC("memmap: reg property value overflow in devicetree node %s",
                  FDT_NODE_NAME(node));
          ADD_MEM_OR_PANIC(((pa_range_t){.start = start, .end = end}));
          break;
        }
      }
    }
    arg->is_memory_done = true;
  } else if (parent && !parent->parent &&
             strcmp(FDT_NODE_NAME(node), "reserved-memory") ==
                 0) { // Current node is /reserved-memory.
    arg->reserved_memory_node = node;
    arg->reserved_memory_n_cells = fdt_get_n_address_size_cells(node);
  } else if (arg->reserved_memory_node) { // The /reserved-memory node has been
                                          // traversed.
    if (parent->node ==
        arg->reserved_memory_node) { // Current node is /reserved-memory/...
      FDT_FOR_ITEM(node, item) {
        if (FDT_TOKEN(item) == FDT_PROP) {
          const fdt_prop_t *const prop = (const fdt_prop_t *)item->payload;
          if (strcmp(FDT_PROP_NAME(prop), "reg") == 0) {
            const fdt_read_reg_result_t read_result =
                fdt_read_reg(prop, arg->root_n_cells);
            const uint32_t start = read_result.value.address,
                           end = start + read_result.value.size;
            if (read_result.address_overflow || read_result.size_overflow ||
                read_result.value.address > UINT32_MAX ||
                read_result.value.size > UINT32_MAX || end < start)
              PANIC("memmap: reg property value overflow in devicetree node %s",
                    FDT_NODE_NAME(node));
            RESERVE_OR_PANIC(
                ((reserved_mem_entry_t){.range = {.start = start, .end = end},
                                        .purpose = RMP_FIRMWARE}));
            break;
          }
        }
      }
    } else { // All children of the /reserved-memory node has been traversed.
      arg->reserved_memory_node = NULL;
      arg->is_reserved_memory_done = true;
    }
  }

  return arg->is_memory_done && arg->is_reserved_memory_done ? CF_BREAK
                                                             : CF_CONTINUE;
}

void scan_mem(void) {
  if (devicetree_is_init()) {
    // Use the devicetree to discover usable and reserved memory regions.

    // Devicetree.
    RESERVE_OR_PANIC(((reserved_mem_entry_t){
        .range = {.start = (pa_t)(uintptr_t)fdt_get_start(),
                  .end = (pa_t)(uintptr_t)fdt_get_end()},
        .purpose = RMP_DTB}));

    // Anything in the memory reservation block.
    for (const fdt_reserve_entry_t *reserve_entry = FDT_START_MEM_RSVMAP;
         !(reserve_entry->address == 0 && reserve_entry->size == 0);
         reserve_entry++) {
      const pa_t start = rev_u64(reserve_entry->address),
                 end = start + rev_u64(reserve_entry->size);
      RESERVE_OR_PANIC(((reserved_mem_entry_t){
          .range = {.start = start, .end = end}, .purpose = RMP_FIRMWARE}));
    }

    // - Usable memory region.
    // - Spin tables for multicore boot.

    scan_mem_fdt_traverse_callback_arg_t arg = {.is_memory_done = false,
                                                .is_reserved_memory_done =
                                                    false,
                                                .reserved_memory_node = NULL};
    fdt_traverse((fdt_traverse_callback_t *)_scan_mem_fdt_traverse_callback,
                 &arg);
  } else {
    // Use hardcoded values as a fallback.

    // Usable memory region.
    ADD_MEM_OR_PANIC(((pa_range_t){.start = 0x0, .end = 0x3b400000}));

    // Spin tables for multicore boot.
    RESERVE_OR_PANIC(((reserved_mem_entry_t){
        .range = {.start = 0x0, .end = 0x1000}, .purpose = RMP_FIRMWARE}));
  }

  // Kernel image in the physical memory.
  RESERVE_OR_PANIC(
      ((reserved_mem_entry_t){.range = {.start = (pa_t)(uintptr_t)_skernel,
                                        .end = (pa_t)(uintptr_t)_ekernel},
                              .purpose = RMP_KERNEL}));

  // Initial ramdisk.
  if (initrd_is_init()) {
    RESERVE_OR_PANIC(((reserved_mem_entry_t){
        .range = {.start = (pa_t)(uintptr_t)initrd_get_start(),
                  .end = (pa_t)(uintptr_t)initrd_get_end()},
        .purpose = RMP_INITRD}));
  }

  // Kernel stack.
  RESERVE_OR_PANIC(
      ((reserved_mem_entry_t){.range = {.start = (pa_t)(uintptr_t)_sstack,
                                        .end = (pa_t)(uintptr_t)_estack},
                              .purpose = RMP_STACK}));

  // Sort the memory regions.
  mem_sort();
  reserved_mem_sort();
}

bool check_memmap(void) {
  size_t ir = 0;
  for (size_t im = 0; im < _n_mems; im++) {
    if (im != 0 &&
        _mems[im - 1].end > _mems[im].start) { // Usable memory region overlaps
                                               // with the previous one.
      return false;
    }

    if (ir < _n_reserved_mems &&
        _reserved_mems[ir].range.start <
            _mems[im].start) { // A reserved memory region lies outside all
                               // usable memory regions.
      return false;
    }

    for (;
         ir < _n_reserved_mems && _reserved_mems[ir].range.end <= _mems[im].end;
         ir++) {
      if (ir != 0 && _reserved_mems[ir - 1].range.end >
                         _reserved_mems[ir]
                             .range.start) { // Reserved memory region overlaps
                                             // with the previous one.
        return false;
      }
    }
  }

  if (ir != _n_reserved_mems) { // A reserved memory region lies outside all
                                // usable memory regions.
    return false;
  }

  return true;
}

void print_memmap(void) {
  console_puts("Memory map:");

  size_t ir = 0;
  for (size_t im = 0; im < _n_mems; im++) {
    console_printf("  Memory region %zu: 0x%8" PRIxPA " - 0x%8" PRIxPA "\n", im,
                   _mems[im].start, _mems[im].end);

    pa_t curr_pa = _mems[im].start;

    for (;
         ir < _n_reserved_mems && _reserved_mems[ir].range.end <= _mems[im].end;
         ir++) {
      if (curr_pa != _reserved_mems[ir].range.start) {
        console_printf("    0x%8" PRIxPA " - 0x%8" PRIxPA ": Usable\n", curr_pa,
                       _reserved_mems[ir].range.start);
      }

      const char *const purpose_str =
          _reserved_mems[ir].purpose == RMP_FIRMWARE ? "firmware"
          : _reserved_mems[ir].purpose == RMP_KERNEL ? "kernel"
          : _reserved_mems[ir].purpose == RMP_INITRD ? "initrd"
          : _reserved_mems[ir].purpose == RMP_DTB    ? "dtb"
          : _reserved_mems[ir].purpose == RMP_STACK  ? "stack"
                                                     : "unknown";
      console_printf("    0x%8" PRIxPA " - 0x%8" PRIxPA ": Reserved: %s\n",
                     _reserved_mems[ir].range.start,
                     _reserved_mems[ir].range.end, purpose_str);

      curr_pa = _reserved_mems[ir].range.end;
    }

    if (curr_pa != _mems[im].end) {
      console_printf("    0x%8" PRIxPA " - 0x%8" PRIxPA ": Usable\n", curr_pa,
                     _mems[im].end);
    }
  }
}

bool memmap_is_usable(const pa_range_t range) {
  bool is_within_any_usable_range = false;
  for (size_t i = 0; i < _n_mems; i++) {
    if (_mems[i].start <= range.start && range.end <= _mems[i].end) {
      is_within_any_usable_range = true;
      break;
    }
  }

  if (!is_within_any_usable_range)
    return false;

  for (size_t i = 0; i < _n_reserved_mems; i++) {
    if (!(_reserved_mems[i].range.end <= range.start ||
          range.end <= _reserved_mems[i].range.start))
      return false;
  }

  return true;
}
