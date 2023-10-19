#include "oscos/uapi/errno.h"

int sys_enosys(void) { return -ENOSYS; }
