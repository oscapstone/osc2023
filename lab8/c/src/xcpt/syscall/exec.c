#include <stddef.h>
#include <stdint.h>

#include "oscos/initrd.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

int sys_exec(const char *const name, char *const argv[const]) {
  // TODO: Handle this.
  (void)argv;

  if (!initrd_is_init()) {
    // Act as if there are no files at all.
    return -ENOENT;
  }

  const cpio_newc_entry_t *const entry = initrd_find_entry_by_pathname(name);
  if (!entry) {
    return -ENOENT;
  }

  const void *user_program_start;
  size_t user_program_len;

  const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
  const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
  if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
    user_program_start = CPIO_NEWC_FILE_DATA(entry);
    user_program_len = CPIO_NEWC_FILESIZE(entry);
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
    return -EISDIR;
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    // TODO: Handle symbolic link.
    return -ELOOP;
  } else { // Unknown file type.
    // Just treat it as an I/O error.
    return -EIO;
  }

  exec(user_program_start, user_program_len);

  // If execution reaches here, then exec failed.
  return -ENOMEM;
}
