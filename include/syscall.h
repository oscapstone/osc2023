#ifndef __SYSCALL_H
#define __SYSCALL_H

#include "type.h"

# define SEEK_SET 0


int sys_get_pid();
uint64_t sys_uartread(char buf[], uint64_t size);
uint64_t sys_uartwrite(const char buf[], uint64_t size);
int sys_exec(const char* name, char *const argv[]);
int sys_fork();
void sys_exit(int status);
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);
void syscall_handler(struct Trapframe_t *frame);
int sys_open(const char *pathname, int flags);

// syscall number : 12
int sys_close(int fd);

// syscall number : 13
// remember to return read size or error code
long sys_write(int fd, const void *buf, unsigned long count);

// syscall number : 14
// remember to return read size or error code
long sys_read(int fd, void *buf, unsigned long count);

// syscall number : 15
// you can ignore mode, since there is no access control
int sys_mkdir(const char *pathname, unsigned mode);

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int sys_mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);

// syscall number : 17
int sys_chdir(const char *path);
long sys_lseek64(int fd, long offset, int whence);
int sys_ioctl(int fd, unsigned long request, void *buf);

#endif