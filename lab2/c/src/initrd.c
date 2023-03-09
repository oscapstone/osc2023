#include "oscos/initrd.h"

static bool _cpio_newc_is_header_field_valid(const char field[const static 8]) {
  for (size_t i = 0; i < 8; i++) {
    if (!(('0' <= field[i] && field[i] <= '9') ||
          ('A' <= field[i] && field[i] <= 'F')))
      return false;
  }
  return true;
}

bool initrd_is_valid(void) {
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
