#ifndef _SCHED_H_
#define _SCHED_H_

#include "u_list.h"

#define PIDMAX 32768
#define USTACK_SIZE 0x10000
#define KSTACK_SIZE 0x10000

extern void  switch_to(void *curr_context, void *next_context);
extern void* get_current();
extern void  store_context(void *curr_context);

typedef struct thread_context
{
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
} thread_context_t;

typedef struct thread
{
    list_head_t listhead;
    thread_context_t context;
    char *data;
    unsigned int datasize;
    int iszombie;
    int pid;
    int isused;
    char* stack_alloced_ptr;
    char* kernel_stack_alloced_ptr;
} thread_t;

void schedule_timer(char *notuse);
void init_thread_sched();
void idle();
void schedule();
void kill_zombies();
void thread_exit();
thread_t *thread_create(void *start);
int exec_thread(char *data, unsigned int filesize);

void foo();

#endif /* _SCHED_H_ */
