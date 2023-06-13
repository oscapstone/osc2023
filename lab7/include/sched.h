#ifndef SCHED_H
#define SCHED_H

#include "list.h"
#include "fs/vfs.h"
#include "syscall.h"

#define PIDMAX 32768
#define USTACK_SIZE 0x4000
#define KSTACK_SIZE 0x4000
#define SIGNAL_MAX  64

extern void switch_to(void *curr_context, void *next_context);
extern void store_context(void *curr_context);
extern void load_context(void *curr_context);
extern void *get_current();

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
    void* ttbr0_el1;   // use for MMU mapping (user space)
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
    char *kernel_stack_alloced_ptr;
    void (*singal_handler[SIGNAL_MAX+1])();
    int sigcount[SIGNAL_MAX + 1];
    void (*curr_signal_handler)();
    int signal_is_checking;
    list_head_t vma_list;
    thread_context_t signal_saved_context;
    char curr_working_dir[MAX_PATH_NAME+1];
    struct file* file_descriptors_table[MAX_FD+1];
} thread_t;

thread_t *curr_thread;
list_head_t *run_queue;
list_head_t *wait_queue;

thread_t threads[PIDMAX + 1];

void schedule_timer(char *notuse);
void init_thread_sched();
void idle();
void schedule();
void kill_zombies();
void thread_exit();
thread_t *thread_create(void *start, unsigned int filesize);
int exec_thread(char *data, unsigned int filesize);

#endif