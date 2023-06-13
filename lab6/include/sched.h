#ifndef _SCHED_H
#define _SCHED_H

#define THREAD_CPU_CONTEXT      0

#ifndef __ASSEMBLER__

#define THREAD_SIZE             4096

#define NR_TASKS                64

#define FIRST_TASK              task[0]
#define LAST_TASK               task[NR_TASKS-1]

#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE	    1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		        3
#define TASK_STOPPED		    4

#define PF_KTHREAD              0x00000002

extern struct task_struct *current;
extern struct task_struct *task[NR_TASKS];
extern int nr_tasks;

struct cpu_context {
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
    unsigned long sp; 
    unsigned long pc;
};

#define MAX_PROCESS_PAGES   128

struct user_page {
    unsigned long phys_addr;
    unsigned long virt_addr;
};

struct mm_struct {
    unsigned long pgd;
    int user_pages_count;
    struct user_page user_pages[MAX_PROCESS_PAGES];
    int kernel_pages_count;
    unsigned long kernel_pages[MAX_PROCESS_PAGES];
};

struct task_struct {
    struct cpu_context cpu_context;
    long state;
    long counter;
    long priority;
    long preempt_count;
    //unsigned long stack; //remove
    unsigned long flags;
    long id;
    struct mm_struct mm;
};

extern void sched_init();
extern void schedule();
extern void timer_tick();
extern void preempt_disable();
extern void preempt_enable();
extern void switch_to(struct task_struct *);
extern void cpu_switch_to(struct task_struct *, struct task_struct *);
extern void exit_process();
extern void kill_zombies();
extern void update_pgd(unsigned long);

#define INIT_TASK \
{ \
{0,0,0,0,0,0,0,0,0,0,0,0,0},\
0, 0, 1, 0, PF_KTHREAD, 0, \
{0,0,{{0}},0,{0}}\
}

#endif

#endif