#include "oscos/fs/vfs.h"

int sys_sync(void) {
  vfs_sync_all();
  return 0;
}
