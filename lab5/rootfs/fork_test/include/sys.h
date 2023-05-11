#ifndef	_SYS_H
#define	_SYS_H

#include "types.h"

#define __NR_syscalls	        9

// #define SYS_WRITE_NUMBER        0 		// syscal numbers 
// #define SYS_UARTWRITE_NUMBER    1
// #define SYS_UARTREAD_NUMBER     2
// #define SYS_GETPID_NUMBER       3
// #define SYS_FORK_NUMBER         4
// #define SYS_EXEC_NUMBER         5
// #define SYS_EXIT_NUMBER         6
// #define SYS_MALLOC_NUMBER       7 	
// #define SYS_CLONE_NUMBER        8

#define SYS_GETPID_NUMBER       0
#define SYS_UARTREAD_NUMBER     1
#define SYS_UARTWRITE_NUMBER    2
#define SYS_EXEC_NUMBER         3
#define SYS_FORK_NUMBER         4
#define SYS_EXIT_NUMBER         5
#define SYS_MBOX_NUMBER	        6
#define SYS_KILL_NUMBER	        7

#define SYS_MALLOC_NUMBER       8 	
#define SYS_CLONE_NUMBER        9
#define SYS_CORETIMER_On        10
#define SYS_CORETIMER_OFF       11
#define SYS_WRITE_NUMBER        12 		// syscal numbers 

#ifndef __ASSEMBLER__

void sys_write(char * buf);
int sys_uart_write(char buf[], size_t size);
int sys_uart_read(char buf[], size_t size);
int sys_gitPID();
int sys_fork();
int sys_exec(const char* name, char* const argv[]);
void sys_exit();
void *sys_malloc(int bytes);
int sys_clone();

// void call_sys_write(char * buf);
// int uartwrite(char buf[], size_t size);
// int uartread(char buf[], size_t size);
// int getpid();
// int fork();
// int exec(const char* name, char* const argv[]);
// void exit();
void *call_sys_malloc();

int getpid();                                       // 0
int uartread(char buf[], size_t size);              // 1
int uartwrite(char buf[], size_t size);             // 2
int exec(const char* name, char* const argv[]);     // 3
int fork();                                         // 4
void exit();                                        // 5
int mbox_call(unsigned char ch, unsigned int *mbox);// 6
void kill(int pid);									// 7

#endif
#endif  /*_SYS_H */