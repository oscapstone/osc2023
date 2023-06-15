#include "stdlib.h"
#include "thread.h"
#include "page_alloc.h"
#include "dynamic_alloc.h"
#include "reserve_mem.h"
#include "list.h"
#include "syscall.h"
#include "vfs.h"

extern task_struct *get_current();
extern void set_switch_timer();
extern void enable_interrupt();
extern void disable_interrupt();
extern void switch_to(task_struct *, task_struct *);
extern void kernel_thread_init();

long thread_cnt = 0;

task_struct kernel_thread = {0};
struct list_head task_rq_head;      // run queue
struct list_head task_zombieq_head; // zombie queue

#define FD_STDIN 0
#define FD_STDOUT 1
#define FD_STDERR 2

void schedule()
{
    task_struct *cur = get_current();
    task_struct *next = del_rq();

    if (next == NULL)
        next = &kernel_thread;
    if (cur != &kernel_thread)
        add_rq(cur);

    set_switch_timer();
    enable_interrupt();

    if (next->status == FORKING)
    {
        add_rq(next);
        switch_to(cur, &kernel_thread);
    }
    else if (next->status == ZOMBIE)
    {
        INIT_LIST_HEAD(&next->list);
        list_add_tail(&next->list, &task_zombieq_head);
        switch_to(cur, &kernel_thread);
    }
    else
    {
        switch_to(cur, next);
    }
}

void add_rq(task_struct *task)
{
    INIT_LIST_HEAD(&task->list);
    list_add_tail(&task->list, &task_rq_head);
}

task_struct *del_rq()
{
    struct list_head *ret;
    ret = task_rq_head.next;
    if (ret != &task_rq_head)
    {
        list_del_init(ret);
        return container_of(ret, task_struct, list);
    }
    else
        return NULL;
}

void thread_init()
{
    INIT_LIST_HEAD(&task_rq_head);
    INIT_LIST_HEAD(&task_zombieq_head);
    kernel_thread_init(&kernel_thread);
    return;
}

thread_info *thread_create(func_ptr fp)
{
    task_struct *new_task = (task_struct *)my_malloc(sizeof(task_struct));
    thread_info *new_thread = (thread_info *)my_malloc(sizeof(thread_info));

    new_task->thread_info = new_thread;
    new_thread->task = new_task;

    new_task->kstack_start = (unsigned long)my_malloc(MIN_PAGE_SIZE);
    new_task->ustack_start = (unsigned long)my_malloc(MIN_PAGE_SIZE);
    new_task->usrpgm_load_addr = USRPGM_BASE + thread_cnt * USRPGM_SIZE;

    new_task->task_context.fp = new_task->kstack_start + MIN_PAGE_SIZE;
    new_task->task_context.lr = (unsigned long)task_wrapper;
    new_task->task_context.sp = new_task->kstack_start + MIN_PAGE_SIZE;
    new_task->thread_info->id = thread_cnt++;
    new_task->status = READY;
    new_task->job = fp;
    new_task->custom_signal = NULL;

    memset(new_task->fd_table, 0, sizeof(new_task->fd_table)); // init file table
    // Open stdin, stdout, stderr for the process
    file_t *fh = NULL;
    vfs_open("/dev/uart", 0, &fh);
    new_task->fd_table[FD_STDIN] = fh;
    vfs_open("/dev/uart", 0, &fh);
    new_task->fd_table[FD_STDOUT] = fh;
    vfs_open("/dev/uart", 0, &fh);
    new_task->fd_table[FD_STDERR] = fh;

    add_rq(new_task);

    return new_thread;
}

/* threads' routine for any task */
void task_wrapper()
{
    task_struct *current = get_current();
    (current->job)();
    exit(0);
}

void idle_task()
{
    while (!list_empty(&task_rq_head) || !list_empty(&task_zombieq_head))
    {
        disable_interrupt();
        kill_zombies();
        do_fork();
        enable_interrupt();
        schedule();
    }
}

/* kill the zombie threads and recycle their resources */
void kill_zombies()
{
    struct list_head *iter = &task_zombieq_head;
    struct list_head *start = &task_zombieq_head;
    while (iter->next != start)
    {
        iter = iter->next;
        task_struct *tmp;
        tmp = container_of(iter, task_struct, list);
        free((void *)tmp->kstack_start);
        free((void *)tmp->ustack_start);
        free(tmp->thread_info);
        if (tmp->custom_signal)
            free(tmp->custom_signal);
        free(tmp);
    }
    INIT_LIST_HEAD(&task_zombieq_head);
    return;
}

void do_fork()
{
    struct list_head *iter = &task_rq_head;
    struct list_head *start = &task_rq_head;
    while (iter->next != start)
    {
        iter = iter->next;
        task_struct *tmp;
        tmp = container_of(iter, task_struct, list);
        if (tmp->status == FORKING)
            create_child(tmp);
    }
}

/* copy all data including stack and program for child process and set the corresponding sp and lr for it*/
void create_child(task_struct *parent)
{
    thread_info *child_thread = thread_create(0);
    task_struct *child = child_thread->task;

    char *parent_d, *child_d;

    parent->status = READY;

    parent->thread_info->child_id = child->thread_info->id;
    child->thread_info->child_id = 0;

    // copy context
    parent_d = (char *)&(parent->task_context);
    child_d = (char *)&(child->task_context);
    for (int i = 0; i < sizeof(context); i++)
        child_d[i] = parent_d[i];

    // copy custom_signal
    if (parent->custom_signal)
    {
        child->custom_signal = (custom_signal *)my_malloc(sizeof(custom_signal));
        parent_d = (char *)&(parent->custom_signal);
        child_d = (char *)&(child->custom_signal);
        for (int i = 0; i < sizeof(custom_signal); i++)
            child_d[i] = parent_d[i];
    }

    // copy kernel stack
    parent_d = (char *)parent->kstack_start;
    child_d = (char *)child->kstack_start;
    for (int i = 0; i < MIN_PAGE_SIZE; i++)
        child_d[i] = parent_d[i];

    // copy user stack
    parent_d = (char *)parent->ustack_start;
    child_d = (char *)child->ustack_start;
    for (int i = 0; i < MIN_PAGE_SIZE; i++)
        child_d[i] = parent_d[i];

    // copy user program
    parent_d = (char *)parent->usrpgm_load_addr;
    child_d = (char *)child->usrpgm_load_addr;
    for (int i = 0; i < USRPGM_SIZE; i++)
        child_d[i] = parent_d[i];

    // set offset to child's stack
    unsigned long kstack_offset = child->kstack_start - parent->kstack_start;
    unsigned long ustack_offset = child->ustack_start - parent->ustack_start;
    unsigned long usrpgm_offset = child->usrpgm_load_addr - parent->usrpgm_load_addr;

    // set child kernel space offset
    child->task_context.fp += kstack_offset;
    child->task_context.sp += kstack_offset;
    child->trapframe = parent->trapframe + kstack_offset;

    // set child user space offset
    trapframe *ctrapframe = (trapframe *)child->trapframe; // because of data type problem
    ctrapframe->x[29] += ustack_offset;
    ctrapframe->sp_el0 += ustack_offset;
    ctrapframe->elr_el1 += usrpgm_offset;
}

void debug_task_rq()
{
    struct list_head *iter;
    struct list_head *start;
    iter = &task_rq_head;
    start = &task_rq_head;
    printf("\n[DEBUG] task run queue\n");
    printf("task_rq_head -> ");
    while (iter->next != start)
    {
        iter = iter->next;
        task_struct *tmp;
        tmp = container_of(iter, task_struct, list);
        printf("thread_id %d -> ", tmp->thread_info->id);
    }
    printf("NULL\n\n");
}

void debug_task_zombieq()
{
    struct list_head *iter;
    struct list_head *start;
    iter = &task_zombieq_head;
    start = &task_zombieq_head;
    printf("\n[DEBUG] task run queue\n");
    printf("task_zombieq_head -> ");
    while (iter->next != start)
    {
        iter = iter->next;
        task_struct *tmp;
        tmp = container_of(iter, task_struct, list);
        printf("thread_id %d -> ", tmp->thread_info->id);
    }
    printf("NULL\n\n");
}

int thread_get_idle_fd(task_struct *thd)
{
    for (int i = 0; i < VFS_PROCESS_MAX_OPEN_FILE; i++)
    {
        if (thd->fd_table[i] == NULL)
            return i;
    }
    return -1;
}