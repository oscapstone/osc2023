#ifndef _SYSCALL_H
#define _SYSCALL_H

#define __NR_SYSCALLS       8

#define SYS_GETPID_NUM      0
#define SYS_UARTREAD_NUM    1
#define SYS_UARTWRITE_NUM   2
#define SYS_EXEC_NUM        3
#define SYS_FORK_NUM        4
#define SYS_EXIT_NUM        5
#define SYS_MBOXCALL_NUM    6
#define SYS_KILL_NUM        7

#ifndef __ASSEMBLER__

int sys_getpid(); 
unsigned sys_uartread(char buf[], unsigned size); 
unsigned sys_uartwrite(const char buf[], unsigned size);
int sys_exec(const char *name, char *const argv[]);
int sys_fork();
void sys_exit(int status); 
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);

#endif

#endif