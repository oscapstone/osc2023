#ifndef SYSCALL_H
#define SYSCALL_H
#include "thread.h"
#include <stddef.h>

int sys_getpid(void);
size_t sys_uart_read(char *, size_t size);
size_t sys_uart_write(const char *, size_t size);
int sys_exec(const char *, char *const argv[]);
int sys_fork(Trap_frame *);
void sys_exit(int status);
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);

int get_pid(void);
size_t uart_read(char *, size_t size);
size_t uart_write(const char *, size_t size);
int exec(const char *, char *const argv[]);
int fork();
void uexit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void kill(int pid);
void sys_signal(int, void (*fn)());
void posix_kill(int, int);

// TEST
void fork_test();
void check_timer();

// Container
void handler_container();

// FS
int sys_open(const char *pathname, int flags);
int sys_close(int fd);
int sys_write(int fd, const void *buf, int count);
int sys_read(int fd, void *, int count);
int sys_mkdir(const char *);
int sys_mount(const char *src, const char *target, const char *filesystem,
              unsigned long, const void *);
int sys_chdir(const char *path);
long sys_lseek64(int fd, long offset, int whence);
int sys_ioctl(int fd, unsigned long request, void* fb_info);

#endif // SYSCALL_H
