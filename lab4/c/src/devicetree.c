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

static const fdt_item_t *_fdt_traverse_rec(
    const fdt_item_t *const node,
    control_flow_t (*const callback)(void *arg, const fdt_item_t *node),
    void *const arg) {
  const control_flow_t result = callback(arg, node);
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
      const fdt_item_t *const next_item =
          _fdt_traverse_rec(item, callback, arg);
      if (!next_item)
        return NULL;
      item = next_item;
    } else {
      item = fdt_next_item(item);
    }
  }

  return item + 1;
}

void fdt_traverse(control_flow_t (*const callback)(void *arg,
                                                   const fdt_item_t *node),
                  void *const arg) {
  const fdt_item_t *const root_node =
      (const fdt_item_t *)(_dtb_start +
                           rev_u32(((const fdt_header_t *)_dtb_start)
                                       ->off_dt_struct));
  _fdt_traverse_rec(root_node, callback, arg);
}
