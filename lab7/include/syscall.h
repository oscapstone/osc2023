#ifndef SYSCALL_H
#define SYSCALL_H

#include "stddef.h"
#include "exception.h"

#define MAX_FD 16

int getpid(trapframe_t *tpf);
size_t uartread(trapframe_t *tpf,char buf[], size_t size);
size_t uartwrite(trapframe_t *tpf,const char buf[], size_t size);
int exec(trapframe_t *tpf,const char *name, char *const argv[]);
int fork(trapframe_t *tpf);
void exit(trapframe_t *tpf,int status);
int syscall_mbox_call(trapframe_t *tpf, unsigned char ch, unsigned int *mbox);
void kill(trapframe_t *tpf,int pid);
void signal_register(int signal, void (*handler)());
void signal_kill(int pid, int signal);
void sigreturn(trapframe_t *tpf);
void *sys_mmap(trapframe_t *tpf, void *addr, size_t len, int prot, int flags, int fd, int file_offset);

// syscall number : 11
int sys_open(trapframe_t *tpf, const char *pathname, int flags);

// syscall number : 12
int sys_close(trapframe_t *tpf, int fd);

// syscall number : 13
// remember to return read size or error code
long sys_write(trapframe_t *tpf, int fd, const void *buf, unsigned long count);

// syscall number : 14
// remember to return read size or error code
long sys_read(trapframe_t *tpf, int fd, void *buf, unsigned long count);

// syscall number : 15
// you can ignore mode, since there is no access control
int sys_mkdir(trapframe_t *tpf, const char *pathname, unsigned mode);

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
int sys_mount(trapframe_t *tpf, const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);

// syscall number : 17
int sys_chdir(trapframe_t *tpf, const char *path);

// syscall number : 18
// you only need to implement seek set
long sys_lseek64(trapframe_t *tpf, int fd, long offset, int whence);

// syscall number : 19
int sys_ioctl(trapframe_t *tpf, int fb, unsigned long request, void *info);

#endif
