#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

struct registers
{
	uint64_t x19;
	uint64_t x20;
	uint64_t x21;
	uint64_t x22;
	uint64_t x23;
	uint64_t x24;
	uint64_t x25;
	uint64_t x26;
	uint64_t x27;
	uint64_t x28;
	uint64_t FP;
	uint64_t LR;
	uint64_t SP;
};

struct thread
{
	struct registers reg;		//save callee-saved register's address
	struct thread *next;
	int tid;
	int status;					//RUN : 1 , DEAD : 2
	int sig;
	void* sig_handler[10];
	char stack[0x10000];
	char* kernel_stack_base;
	char* kernel_stack;
};

#endif

void init_thread();
int Thread(void (*func)());
void push_run_queue(struct thread *thd);
void pop_run_queue(int tid);
void schedule();
void kill_zombies();
void idle();
void push_idle();
void repush_idle();
int get_tid();
void clear_thread_list();
void foo();
void exit();
void* video_prog();
