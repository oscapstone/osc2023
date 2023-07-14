#include <stddef.h>
#include <stdint.h>

#include "oscos/initrd.h"
#include "oscos/sched.h"
#include "oscos/uapi/errno.h"

int sys_exec(const char *const name, char *const argv[const]) {
  // TODO: Handle this.
  (void)argv;

  struct file *user_program_file;
  const int open_result = vfs_open(name, 0, &user_program_file);
  if (open_result < 0)
    return open_result;

  exec(user_program_file);

  // If execution reaches here, then exec failed.
  return -ENOMEM;
}
