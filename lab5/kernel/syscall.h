#ifndef SYSCALL_H
#define SYSCALL_H

/*
 * System calls
 */
extern int getpid(void);
extern unsigned int uartread(char buf[], unsigned int size);
extern unsigned int uartwrite(const char buf[], unsigned int size);
extern int exec(const char *name, char *const argv[]);
extern int fork(void);
extern void exit(int status);
extern int mbox_call(unsigned char ch, unsigned int *mbox);
extern void kill(int pid);

/*
 * Handelers
 */
void syscall_enterance(void);

/*
 * Demo
 */
void demo_syscall(void);

#endif /* SYSCALL_H */