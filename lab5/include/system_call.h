#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

#include <stdint.h>

struct trap_frame
{
	uint64_t reg[31];
	uint64_t SP_EL0;
	uint64_t SPSR_EL1;
	uint64_t ELR_EL1;
};

#endif

void EL0_SVC_handler();
int getpid();
int uart_read(char* buffer,int size);
int uart_write(char* buffer,int size);
int exec(char *name,char *argv);
int fork(struct trap_frame *tf);
void kill(int pid);
void fork_test();
int test_get_pid();
int test_fork();
void test_exit();
