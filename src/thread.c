#include "thread.h"
#include "mm.h"
#include "exception.h"
#include "uart.h"
#include "list.h"
pid_t pid_cnter;
LIST_HEAD(running_queue);
LIST_HEAD(waiting_queue);
LIST_HEAD(stop_queue);
static void real_thread()
{
    task_t *current = get_current_thread();
    ((void (*)())(current->text))();
    exit(0);
}

task_t *new_thread()
{
    // disable_local_all_interrupt();
    //TODO
    //nested disable interrupt
    task_t *new_thread_ptr = (task_t *)kmalloc(sizeof(task_t));
    INIT_LIST_HEAD(&(new_thread_ptr->node));
    new_thread_ptr->state = TASK_RUNNING;
    new_thread_ptr->kernel_stack = (char *)alloc_pages(1);
    new_thread_ptr->kernel_stack_size = PAGE_SIZE;
    new_thread_ptr->tid = pid_cnter++;
    // new_thread_ptr->ppid = get_current_thread()->tgid;
    new_thread_ptr->user_stack = (char *)alloc_pages(1);
    new_thread_ptr->user_stack_size = PAGE_SIZE;
    my_bzero(&(new_thread_ptr->old_reg_set), sizeof(new_thread_ptr->old_reg_set));
    new_thread_ptr->old_reg_set.lr = (uint64_t)real_thread;
    new_thread_ptr->old_reg_set.sp = new_thread_ptr->kernel_stack + PAGE_SIZE;//stack is decremental
    new_thread_ptr->exit_code = new_thread_ptr->exit_state = new_thread_ptr->exit_signal = 0;
    return new_thread_ptr;
}

task_t *create_thread(char *text)
{
    task_t *new_thread_ptr = new_thread();
    new_thread_ptr->text = text;
    // new_thread_ptr->old_reg_set.lr = (uint64_t)text;
    //insert to tail of running queue
    list_add_tail(&(new_thread_ptr->node), &running_queue);
    uart_write_string("new thread: 0x");
    uart_write_no_hex((uint64_t)new_thread_ptr);
    uart_write_string("\n");
    return new_thread_ptr;
}

void destruct_thread(task_t *t)
{
    free_page(t->kernel_stack);
    free_page(t->user_stack);
    kfree(t);
}

void schedule()
{
    task_t *next_task = list_entry(running_queue.next, task_t, node);
    list_del(&(next_task->node));
    task_t *current = get_current_thread();
    if ((current->state | current->exit_state) == TASK_RUNNING)
        list_add_tail(&(current->node), &running_queue);
    //context switch
    switch_to(&(current->old_reg_set), &(next_task->old_reg_set));
}

void exit(int exitcode)
{
    uart_write_string("call exit\n");
    task_t *current = get_current_thread();
    current->exit_code = exitcode;
    current->state = 0;
    current->exit_state = EXIT_ZOMBIE;
    list_del(current);
    list_add_tail(current, &stop_queue);
    schedule();
}

void kill_zombies()
{
    // uart_write_string("in kill zombies\n");
    //kill all threads in stop queue
    task_t *safe;
    task_t *p;
    list_for_each_entry_safe(p, safe, &stop_queue, node) {
        uart_write_string("killing a zombie\n");
        list_del(&(p->node));
        destruct_thread(p);
    }
}
extern struct task_reg_set *get_current_ctx();
task_t *get_current_thread()
{
    struct task_reg_set *current = get_current_ctx();
    return container_of(current, task_t, old_reg_set);
}

void idle_thread()
{
    while (1) {
        //no runnable task, idle
        while (list_empty(&running_queue))
            kill_zombies();
        schedule();
    }
}

void thread_demo()
{
    task_t *current;
    for (int i = 0; i < 10; i++) {
        current = get_current_thread();
        uart_write_string("Current: 0x");
        uart_write_no_hex((uint64_t)current);
        uart_write_string("\nThread id: ");
        uart_write_no(current->tid);
        uart_write_string(" ");
        uart_write_no(i);
        uart_write_string("\n");
        delay(1000000);
        schedule();
    }
    current = get_current_thread();
    uart_write_string("Current: 0x");
    uart_write_no_hex((uint64_t)current);
    uart_write_string("\nThread id: ");
    uart_write_no(current->tid);
    uart_write_string(" end\n");
}

void demo_thread()
{
    // create_thread(idle_thead);
    for (int i = 0; i < 4; i++) {
        create_thread(thread_demo);
    }
    task_t *idle_thread_ptr = new_thread();
    idle_thread_ptr->text = idle_thread;
    idle_thread_ptr->old_reg_set.lr = (uint64_t)idle_thread;
    idle_thread_ptr->state = TASK_RUNNING;
    write_sysreg(tpidr_el1, &(idle_thread_ptr->old_reg_set));
    idle_thread();
    // schedule();
}