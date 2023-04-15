#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <stddef.h>

int    getpid();
size_t uartread(char buf[], size_t size);
size_t uartwrite(const char buf[], size_t size);
int    exec(const char *name, char *const argv[]);
int    fork();
void   exit(int status);
int    mbox_call(unsigned char ch, unsigned int *mbox);
void   kill(int pid);

unsigned int get_file_size(char *thefilepath);
char *get_file_start(char *thefilepath);

#endif /* _SYSCALL_H_*/
