#ifndef OSCOS_USER_PROGRAM_LIBC_SYS_STAT_H
#define OSCOS_USER_PROGRAM_LIBC_SYS_STAT_H

typedef unsigned mode_t;

int mkdir(const char *pathname, mode_t mode);

#endif
