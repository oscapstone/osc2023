#include "oscos/initrd.h"

#include "oscos/devicetree.h"

static const void *_initrd_start, *_initrd_end;

static bool _cpio_newc_is_header_field_valid(const char field[const static 8]) {
  for (size_t i = 0; i < 8; i++) {
    if (!(('0' <= field[i] && field[i] <= '9') ||
          ('A' <= field[i] && field[i] <= 'F')))
      return false;
  }
  return true;
}

static bool _initrd_is_valid(void) {
  const cpio_newc_entry_t *entry;

  // Cannot use INITRD_FOR_ENTRY here, since it will evaluate
  // `CPIO_NEWC_IS_ENTRY_LAST(entry)` before `entry` is validated.
  for (entry = INITRD_HEAD;; entry = CPIO_NEWC_NEXT_ENTRY(entry)) {
    if (entry >= (cpio_newc_entry_t *)_initrd_end)
      return false;
    if (!(strncmp(entry->header.c_magic, "070701", 6) == 0 &&
          _cpio_newc_is_header_field_valid(entry->header.c_mode) &&
          _cpio_newc_is_header_field_valid(entry->header.c_uid) &&
          _cpio_newc_is_header_field_valid(entry->header.c_gid) &&
          _cpio_newc_is_header_field_valid(entry->header.c_nlink) &&
          _cpio_newc_is_header_field_valid(entry->header.c_mtime) &&
          _cpio_newc_is_header_field_valid(entry->header.c_filesize) &&
          _cpio_newc_is_header_field_valid(entry->header.c_devmajor) &&
          _cpio_newc_is_header_field_valid(entry->header.c_devminor) &&
          _cpio_newc_is_header_field_valid(entry->header.c_rdevmajor) &&
          _cpio_newc_is_header_field_valid(entry->header.c_rdevminor) &&
          _cpio_newc_is_header_field_valid(entry->header.c_namesize) &&
          _cpio_newc_is_header_field_valid(entry->header.c_check)))
      return false;

    if (CPIO_NEWC_IS_ENTRY_LAST(entry))
      break;
  }

  return CPIO_NEWC_NEXT_ENTRY(entry) <= (cpio_newc_entry_t *)_initrd_end;
}

typedef struct {
  bool start_done, end_done;
} initrd_init_dtb_traverse_callback_arg_t;

static control_flow_t _initrd_init_dtb_traverse_callback(
    initrd_init_dtb_traverse_callback_arg_t *const arg,
    const fdt_item_t *const node,
    const fdt_traverse_parent_list_node_t *const parent) {
  if (parent && !parent->parent &&
      strcmp(FDT_NODE_NAME(node), "chosen") == 0) { // Current node is /chosen.
    FDT_FOR_ITEM(node, item) {
      if (FDT_TOKEN(item) == FDT_PROP) {
        const fdt_prop_t *const prop = (const fdt_prop_t *)item->payload;
        if (strcmp(FDT_PROP_NAME(prop), "linux,initrd-start") == 0) {
          const uint32_t adr = rev_u32(*(const uint32_t *)FDT_PROP_VALUE(prop));
          _initrd_start = (const void *)(uintptr_t)adr;
          arg->start_done = true;
        } else if (strcmp(FDT_PROP_NAME(prop), "linux,initrd-end") == 0) {
          const uint32_t adr = rev_u32(*(const uint32_t *)FDT_PROP_VALUE(prop));
          _initrd_end = (const void *)(uintptr_t)adr;
          arg->end_done = true;
        }

        if (arg->start_done && arg->end_done)
          break;
      }
    }
    return CF_BREAK;
  } else {
    return CF_CONTINUE;
  }
}

bool initrd_init(void) {
  _initrd_start = NULL;

  if (devicetree_is_init()) {
    // Discover the initrd loading address through the devicetree.

    initrd_init_dtb_traverse_callback_arg_t arg = {.start_done = false,
                                                   .end_done = false};
    fdt_traverse((fdt_traverse_callback_t *)_initrd_init_dtb_traverse_callback,
                 &arg);
    if (!(arg.start_done &&
          arg.end_done)) { // Either the /chosen/linux,initrd-start or the
                           // /chosen/linux,initrd-end property is missing from
                           // the devicetree.
      _initrd_start = NULL;
    }
  }

  // Validate the initial ramdisk.
  if (!(_initrd_start && _initrd_is_valid())) {
    _initrd_start = NULL;
  }

  return _initrd_start;
}

bool initrd_is_init(void) { return _initrd_start; }

const void *initrd_get_start(void) { return _initrd_start; }

const void *initrd_get_end(void) { return _initrd_end; }

uint32_t cpio_newc_parse_header_field(const char field[static 8]) {
  uint32_t result = 0;
  for (size_t i = 0; i < 8; i++) {
    const uint32_t digit_value =
        '0' <= field[i] && field[i] <= '9'   ? field[i] - '0'
        : 'A' <= field[i] && field[i] <= 'F' ? field[i] - 'A' + 10
                                             : (__builtin_unreachable(), 0);
    result = result << 4 | digit_value;
  }
  return result;
}

const cpio_newc_entry_t *initrd_find_entry_by_pathname(const char *pathname) {
  INITRD_FOR_ENTRY(entry) {
    if (strcmp(CPIO_NEWC_PATHNAME(entry), pathname) == 0) {
      return entry;
    }
  }
  return NULL;
}
