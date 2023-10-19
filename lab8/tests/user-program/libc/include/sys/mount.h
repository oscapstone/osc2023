#ifndef OSCOS_USER_PROGRAM_LIBC_SYS_MOUNT_H
#define OSCOS_USER_PROGRAM_LIBC_SYS_MOUNT_H

int mount(const char *source, const char *target, const char *filesystemtype,
          unsigned long mountflags, const void *data);

#endif
