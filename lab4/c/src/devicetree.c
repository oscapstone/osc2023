#include "oscos/devicetree.h"

#include <stddef.h>

static const char *_dtb_start = NULL;

bool devicetree_init(const void *const dtb_start) {
  // TODO: More thoroughly validate the DTB.

  if (rev_u32(((const fdt_header_t *)dtb_start)->magic) == 0xd00dfeed) {
    _dtb_start = dtb_start;
    return true;
  } else {
    _dtb_start = NULL;
    return false;
  }
}

bool devicetree_is_init(void) { return _dtb_start; }

const char *fdt_get_start(void) { return _dtb_start; }

const char *fdt_get_end(void) {
  return _dtb_start + rev_u32(((const fdt_header_t *)_dtb_start)->totalsize);
}

const fdt_item_t *fdt_next_item(const fdt_item_t *const item) {
  switch (FDT_TOKEN(item)) {
  case FDT_BEGIN_NODE: {
    const fdt_item_t *curr;
    FDT_FOR_ITEM_(item, curr);
    return curr + 1;
  }
  case FDT_PROP: {
    const fdt_prop_t *const prop = (const fdt_prop_t *)(item->payload);
    return (const fdt_item_t *)ALIGN(
        (uintptr_t)(prop->value + rev_u32(prop->len)), 4);
  }
  case FDT_NOP:
    return item + 1;
  default:
    __builtin_unreachable();
  }
}

static const fdt_item_t *
_fdt_traverse_rec(const fdt_item_t *const node,
                  const fdt_traverse_parent_list_node_t *const parent,
                  fdt_traverse_callback_t *const callback, void *const arg) {
  const control_flow_t result = callback(arg, node, parent);
  if (result == CF_CONTINUE) {
    // No-op.
  } else if (result == CF_BREAK) {
    return NULL;
  } else {
    __builtin_unreachable();
  }

  const fdt_item_t *item;
  for (item = FDT_ITEMS_START(node); !FDT_ITEM_IS_END(item);) {
    if (FDT_TOKEN(item) == FDT_BEGIN_NODE) {
      const fdt_traverse_parent_list_node_t next_parent = {.node = node,
                                                           .parent = parent};
      const fdt_item_t *const next_item =
          _fdt_traverse_rec(item, &next_parent, callback, arg);
      if (!next_item)
        return NULL;
      item = next_item;
    } else {
      item = fdt_next_item(item);
    }
  }

  return item + 1;
}

void fdt_traverse(fdt_traverse_callback_t *const callback, void *const arg) {
  const fdt_item_t *const root_node =
      (const fdt_item_t *)(_dtb_start +
                           rev_u32(((const fdt_header_t *)_dtb_start)
                                       ->off_dt_struct));
  _fdt_traverse_rec(root_node, NULL, callback, arg);
}

fdt_n_address_size_cells_t
fdt_get_n_address_size_cells(const fdt_item_t *const node) {
  uint32_t n_address_cells = 2, n_size_cells = 1;
  bool n_address_cells_done = false, n_size_cells_done = false;
  FDT_FOR_ITEM(node, item) {
    if (FDT_TOKEN(item) == FDT_PROP) {
      const fdt_prop_t *const prop = (const fdt_prop_t *)item->payload;
      if (strcmp(FDT_PROP_NAME(prop), "#address-cells") == 0) {
        n_address_cells = rev_u32(*(const uint32_t *)FDT_PROP_VALUE(prop));
        n_address_cells_done = true;
      } else if (strcmp(FDT_PROP_NAME(prop), "#size-cells") == 0) {
        n_size_cells = rev_u32(*(const uint32_t *)FDT_PROP_VALUE(prop));
        n_size_cells_done = true;
      }
    }
    if (n_address_cells_done && n_size_cells_done)
      break;
  }

  return (fdt_n_address_size_cells_t){.n_address_cells = n_address_cells,
                                      .n_size_cells = n_size_cells};
}

fdt_read_reg_result_t fdt_read_reg(const fdt_prop_t *const prop,
                                   const fdt_n_address_size_cells_t n_cells) {
  const uint32_t *arr = (const uint32_t *)FDT_PROP_VALUE(prop);

  uintmax_t address = 0;
  bool address_overflow = false;
  for (uint32_t i = 0; i < n_cells.n_address_cells; i++) {
    address_overflow = address_overflow || (address >> 32) != 0;
    address = address << 32 | rev_u32(*arr++);
  }

  uintmax_t size = 0;
  bool size_overflow = false;
  for (uint32_t i = 0; i < n_cells.n_size_cells; i++) {
    size_overflow = size_overflow || (size >> 32) != 0;
    size = size << 32 | rev_u32(*arr++);
  }

  return (fdt_read_reg_result_t){.value = {.address = address, .size = size},
                                 .address_overflow = address_overflow,
                                 .size_overflow = size_overflow};
}
