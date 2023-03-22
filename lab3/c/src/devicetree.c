#include "oscos/devicetree.h"

#include <stddef.h>

static const char *_dtb_start = NULL;

void devicetree_register(const void *const dtb_start) {
  // TODO: Validate DTB.
  _dtb_start = dtb_start;
}

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

void fdt_traverse(control_flow_t (*const callback)(void *arg,
                                                   const fdt_item_t *node),
                  void *const arg) {
  const fdt_item_t *const root_node =
      (const fdt_item_t *)(_dtb_start +
                           rev_u32(((const fdt_header_t *)_dtb_start)
                                       ->off_dt_struct));
  FDT_FOR_ITEM(root_node, item) {
    if (FDT_TOKEN(item) == FDT_BEGIN_NODE) {
      const control_flow_t result = callback(arg, item);
      if (result == CF_CONTINUE) {
        // No-op.
      } else if (result == CF_BREAK) {
        break;
      } else {
        __builtin_unreachable();
      }
    }
  }
}
