#ifndef SCHED_H
#define SCHED_H

#include "list.h"
#include "type.h"

#define PIDMAX 32768
#define USTACK_SIZE 0x10000
#define KSTACK_SIZE 0x10000
#define SIGNAL_MAX 64

typedef void (*signal_handler_t)(void);

typedef enum thread_status {
    FREE,
    READY,
    DEAD,
} thread_status_t;

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
    /* ready queue, wait queue */
    list_head_t listhead;

    /* Context*/
    thread_context_t context;

    /* Data info */
    char *data;
    unsigned int datasize;

    /* Thread status*/
    int id;
    thread_status_t status;

    /* Stack pointers*/
    char *user_sp;
    char *kernel_sp;

    /* Signal */
    signal_handler_t signal_handler[SIGNAL_MAX + 1];
    int sigcount[SIGNAL_MAX + 1];
    signal_handler_t cur_signal_handler;
    int signal_is_checking;
    thread_context_t signal_saved_context;
} thread_t;

extern list_head_t * ready_queue;
extern list_head_t * wait_queue;
extern thread_t * cur_thread;
extern thread_t threads[PIDMAX + 1];


void init_sched_thread();
thread_t * thread_create(void * start);
int thread_exec(char * data, unsigned int filesize);
void thread_exit();
void schedule();
void kill_zombies();
void idle();
void schedule_timer(char * notuse);

#endif
