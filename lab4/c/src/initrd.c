#include "oscos/initrd.h"

#include "oscos/devicetree.h"

static const void *_initrd_start;

static bool _cpio_newc_is_header_field_valid(const char field[const static 8]) {
  for (size_t i = 0; i < 8; i++) {
    if (!(('0' <= field[i] && field[i] <= '9') ||
          ('A' <= field[i] && field[i] <= 'F')))
      return false;
  }
  return true;
}

static bool _initrd_is_valid(void) {
  // Cannot use INITRD_FOR_ENTRY here, since it will evaluate
  // `CPIO_NEWC_IS_ENTRY_LAST(entry)` before `entry` is validated.
  for (const cpio_newc_entry_t *entry = INITRD_HEAD;;
       entry = CPIO_NEWC_NEXT_ENTRY(entry)) {
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

  return true;
}

static control_flow_t
_initrd_init_dtb_traverse_callback(void *const _arg,
                                   const fdt_item_t *const node) {
  (void)_arg;

  if (strcmp(FDT_NODE_NAME(node), "chosen") == 0) {
    FDT_FOR_ITEM(node, item) {
      if (FDT_TOKEN(item) == FDT_PROP) {
        const fdt_prop_t *const prop = (const fdt_prop_t *)item->payload;
        if (strcmp(FDT_PROP_NAME(prop), "linux,initrd-start") == 0) {
          const uint32_t adr = rev_u32(*(const uint32_t *)FDT_PROP_VALUE(prop));
          _initrd_start = (const void *)(uintptr_t)adr;
          break;
        }
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
    fdt_traverse(_initrd_init_dtb_traverse_callback, NULL);
  }

  return _initrd_start && _initrd_is_valid();
}

bool initrd_is_init(void) { return _initrd_start; }

const void *initrd_get_start(void) { return _initrd_start; }

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
