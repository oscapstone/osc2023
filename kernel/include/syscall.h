#ifndef SYSCALL_H
#define SYSCALL_H

#include "stddef.h"
#include "trapframe.h"
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


int sys_open(trapframe_t *tpf, const char *pathname, int flags);
int sys_close(trapframe_t *tpf, int fd);
long sys_write(trapframe_t *tpf, int fd, const void *buf, unsigned long count);
long sys_read(trapframe_t *tpf, int fd, void *buf, unsigned long count);
int sys_mkdir(trapframe_t *tpf, const char *pathname, unsigned mode);
int sys_mount(trapframe_t *tpf, const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
int sys_chdir(trapframe_t *tpf, const char *path);
long sys_lseek64(trapframe_t *tpf, int fd, long offset, int whence);
int sys_ioctl(trapframe_t *tpf, int fb, unsigned long request, void *info);
#endif