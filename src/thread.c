#include "thread.h"
#include "mm.h"
#include "exception.h"
#include "uart.h"
#include "list.h"
#include "exception.h"

pid_t pid_cnter;
task_t *tid2task[MAX_TASK_CNT];

LIST_HEAD(running_queue);
LIST_HEAD(waiting_queue);
LIST_HEAD(stop_queue);
static void real_thread()
{
    task_t *current = get_current_thread();
    ((void (*)())(current->text))();
    _exit(0);
}

task_t *_new_thread()
{
    task_t *new_thread_ptr = (task_t *)kmalloc(sizeof(task_t));
    INIT_LIST_HEAD(&(new_thread_ptr->node));
    new_thread_ptr->state = TASK_RUNNING;
    new_thread_ptr->kernel_stack = (char *)alloc_pages(1);
    new_thread_ptr->kernel_stack_size = PAGE_SIZE;
    
    // new_thread_ptr->ppid = get_current_thread()->tgid;
    new_thread_ptr->user_stack = (char *)alloc_pages(1);
    new_thread_ptr->user_stack_size = PAGE_SIZE;
    my_bzero(&(new_thread_ptr->old_reg_set), sizeof(new_thread_ptr->old_reg_set));
    new_thread_ptr->old_reg_set.lr = (uint64_t)real_thread;
    //stack is decremental
    new_thread_ptr->old_reg_set.sp = (uint64_t)STACK_BASE(new_thread_ptr->kernel_stack, new_thread_ptr->kernel_stack_size);
    new_thread_ptr->exit_code = new_thread_ptr->exit_state = new_thread_ptr->exit_signal = 0;
    return new_thread_ptr;
}

pid_t next_free_tid()
{
    while (tid2task[pid_cnter]) {
        pid_cnter = (pid_cnter + 1) % MAX_TASK_CNT;
    }
    return pid_cnter;
}

task_t *new_thread()
{
    task_t *new_thread_ptr = _new_thread();
    new_thread_ptr->tid = next_free_tid();
    tid2task[new_thread_ptr->tid] = new_thread_ptr;
    return new_thread_ptr;
}
extern void return_from_fork();
task_t *copy_thread(task_t *src)
{
    task_t *dst = new_thread();
    if (src->kernel_stack_size != dst->kernel_stack_size) {
        free_page(dst->kernel_stack);
        dst->kernel_stack = alloc_pages(dst->kernel_stack_size / PAGE_SIZE);
        dst->kernel_stack_size = src->kernel_stack_size;
    }
    if (src->user_stack_size != dst->user_stack_size) {
        free_page(dst->user_stack);
        dst->user_stack = alloc_pages(dst->user_stack_size / PAGE_SIZE);
        dst->user_stack_size = src->user_stack_size;
    }
    //unnecessary
    // dst->text = src->text;
    memcpy(&(dst->old_reg_set), &(src->old_reg_set), sizeof(struct task_reg_set));
    //copy kernel stack
    char *stk_base = STACK_BASE(src->kernel_stack, src->kernel_stack_size);
    char *stk_top = (char *)src->tf;
    size_t offset = stk_base - stk_top;
    //copy from top -> base
    memcpy(STACK_BASE(dst->kernel_stack, dst->kernel_stack_size) - offset, stk_top, offset);
    //update kernel stack pointer which will be loaded on context switching
    dst->old_reg_set.sp = STACK_BASE(dst->kernel_stack, dst->kernel_stack_size) - offset;

    struct trap_frame *dst_tf = (struct trap_frame *)dst->old_reg_set.sp;
    struct trap_frame *src_tf = (struct trap_frame *)src->tf;
    dst->tf = dst_tf;
    // write in syscall_fork
    // src_tf->gprs[0] = dst->tid;
    dst_tf->gprs[0] = 0;

    //copy user stack
    // memcpy(dst->user_stack, src->user_stack, src->user_stack_size);
    stk_base = STACK_BASE(src->user_stack, src->user_stack_size);//src->user_stack + src->user_stack_size;
    //user stack pointer is in the trap frame
    stk_top = (char *)src_tf->sp;
    offset = stk_base - stk_top;
    memcpy(STACK_BASE(dst->user_stack, dst->user_stack_size) - offset, stk_top, offset);
    //update user stack pointer
    dst_tf->sp = (uint64_t)STACK_BASE(dst->user_stack, dst->user_stack_size) - offset;
    
    dst->exit_state = src->exit_state;
    dst->exit_code = src->exit_code;
    dst->exit_signal = src->exit_signal;
    
    dst->old_reg_set.lr = return_from_fork;
    return dst;
}

task_t *copy_run_thread(task_t *src)
{
    task_t *dst = copy_thread(src);
    //disable_interrupt();
    list_add_tail(&(dst->node), &running_queue);
    //test_enable_interrupt();
    return dst;
}

task_t *create_thread(char *text)
{
    task_t *new_thread_ptr = new_thread();
    new_thread_ptr->text = text;
    // new_thread_ptr->old_reg_set.lr = (uint64_t)text;
    //insert to tail of running queue
    //disable_interrupt();
    list_add_tail(&(new_thread_ptr->node), &running_queue);
    //test_enable_interrupt();
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
    disable_interrupt();
    // uart_write_string("schedule disable irq ");
    // uart_write_no(interrupt_cnter);
    // uart_write_string("\n");
    if (!list_empty(&running_queue)) {
        task_t *next_task = list_entry(running_queue.next, task_t, node);
        list_del_init(&(next_task->node));
        task_t *current = get_current_thread();
        if ((current->state | current->exit_state) == TASK_RUNNING)
            list_add_tail(&(current->node), &running_queue);
        // uart_write_string("leave schedule enable irq ");
        // uart_write_no(interrupt_cnter);
        // uart_write_string("\n");
        test_enable_interrupt();
        //context switch
        switch_to(&(current->old_reg_set), &(next_task->old_reg_set));
    }
}

void _exit(int exitcode)
{
    uart_write_string("call _exit\n");
    task_t *current = get_current_thread();
    current->exit_code = exitcode;
    current->state = 0;
    current->exit_state = EXIT_ZOMBIE;
    disable_interrupt();
    list_del_init(current);
    list_add_tail(current, &stop_queue);
    test_enable_interrupt();
    schedule();
}

void kill_zombies()
{
    // uart_write_string("in kill zombies\n");
    //kill all threads in stop queue
    task_t *safe;
    task_t *p;
    disable_interrupt();
    list_for_each_entry_safe(p, safe, &stop_queue, node) {
        uart_write_string("killing a zombie\n");
        tid2task[p->tid] = NULL;
        list_del(&(p->node));
        destruct_thread(p);
    }
    test_enable_interrupt();
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
        disable_interrupt();
        if (list_empty(&stop_queue))
            kill_zombies();
        test_enable_interrupt();
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

void init_idle_thread()
{
    task_t *idle_thread_ptr = _new_thread();
    free_page(idle_thread_ptr->kernel_stack);
    idle_thread_ptr->kernel_stack_size = PAGE_SIZE;
    idle_thread_ptr->kernel_stack = LOW_MEMORY - PAGE_SIZE;
    idle_thread_ptr->text = idle_thread;
    idle_thread_ptr->old_reg_set.lr = (uint64_t)idle_thread;
    idle_thread_ptr->state = TASK_RUNNING;
    idle_thread_ptr->tid = next_free_tid();
    tid2task[idle_thread_ptr->tid] = idle_thread_ptr;
    write_sysreg(tpidr_el1, &(idle_thread_ptr->old_reg_set));
}

void demo_thread()
{
    for (int i = 0; i < 4; i++) {
        create_thread(thread_demo);
    }
}

void time_reschedule(void *data)
{
    if (!list_empty(&running_queue)) {
        disable_interrupt();
        char *ret_addr = read_sysreg(elr_el1);
        asm volatile("mov x30, %0" : : "r" (ret_addr));
        test_enable_interrupt();
        schedule();
    }
}