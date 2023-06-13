#ifndef _SYSCALL_H
#define _SYSCALL_H

#define __NR_SYSCALLS       18

#define SYS_GETPID_NUM      0
#define SYS_UARTREAD_NUM    1
#define SYS_UARTWRITE_NUM   2
#define SYS_EXEC_NUM        3
#define SYS_FORK_NUM        4
#define SYS_EXIT_NUM        5
#define SYS_MBOXCALL_NUM    6
#define SYS_KILL_NUM        7

#define SYS_OPEN_NUM        11
#define SYS_CLOSE_NUM       12
#define SYS_WRITE_NUM       13
#define SYS_READ_NUM        14
#define SYS_MKDIR_NUM       15
#define SYS_MOUNT_NUM       16
#define SYS_CHDIR_NUM       17

#ifndef __ASSEMBLER__

int getpid(); 
unsigned uart_read(char buf[], unsigned size); 
unsigned uart_write(const char buf[], unsigned size);
int exec(const char *name, char *const argv[]);
int fork();
void exit(int status); 
int mbox_call(unsigned char ch, volatile unsigned int *mbox);
void kill(int pid);
int open(const char *pathname, int flags);
int close(int fd);
long write(int fd, const void *buf, unsigned long count);
long read(int fd, void *buf, unsigned long count);
int mkdir(const char *pathname, unsigned mode);
int mount(const char *src, const char *target, const char *fs, unsigned long flags, const void *data);
int chdir(const char *path);

int sys_getpid(); 
unsigned sys_uartread(char buf[], unsigned size); 
unsigned sys_uartwrite(const char buf[], unsigned size);
int sys_exec(const char *name, char *const argv[]);
int sys_fork();
void sys_exit(int status); 
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);
int sys_open(const char *pathname, int flags);
int sys_close(int fd);
long sys_write(int fd, const void *buf, unsigned long count);
long sys_read(int fd, void *buf, unsigned long count);
int sys_mkdir(const char *pathname, unsigned mode);
int sys_mount(const char *src, const char *target, const char *fs, unsigned long flags, const void *data);
int sys_chdir(const char *path);

#endif

#endif