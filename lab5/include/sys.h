#ifndef	_SYS_H
#define	_SYS_H

#include "types.h"

#define __NR_syscalls	        11

// #define SYS_WRITE_NUMBER        0 		// syscal numbers 
// #define SYS_UARTWRITE_NUMBER    1
// #define SYS_UARTREAD_NUMBER     2
// #define SYS_GETPID_NUMBER       3
// #define SYS_FORK_NUMBER         4
// #define SYS_EXEC_NUMBER         5
// #define SYS_EXIT_NUMBER         6
// #define SYS_MALLOC_NUMBER       7 	
// #define SYS_CLONE_NUMBER        8
// #define SYS_CORETIMER_On        9
// #define SYS_CORETIMER_OFF       10

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

int sys_gitPID();                                       // 0
int sys_uart_read(char buf[], size_t size);             // 1
int sys_uart_write(char buf[], size_t size);            // 2
int sys_exec(const char* name, char* const argv[]);     // 3
int sys_fork();                                         // 4
void sys_exit();                                        // 5
int sys_mbox_call(unsigned char ch, unsigned int *mbox);// 6
void sys_kill(int pid);									// 7

void *sys_malloc(int bytes);
void sys_write(char * buf);
int sys_clone();
void sys_coreTimer_on();
void sys_coreTimer_off();

int getpid();                                       // 0
int uartread(char buf[], size_t size);              // 1
int uartwrite(char buf[], size_t size);             // 2
int exec(const char* name, char* const argv[]);     // 3
int fork();                                         // 4
void exit();                                        // 5
int mbox_call(unsigned char ch, unsigned int *mbox);// 6
void kill(int pid);									// 7

void *call_sys_malloc();
void call_sys_write(char * buf);
void call_sys_coreTimer_on();
void call_sys_coreTimer_off();

#endif
#endif  /*_SYS_H */