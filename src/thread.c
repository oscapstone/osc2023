#include "thread.h"
#include "mm.h"
#include "exception.h"
#include "uart.h"
#include "list.h"
#include "exception.h"
#include "signal.h"
#include "vfs.h"

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
    uart_write_string("in _new_thread\n");
    task_t *new_thread_ptr = (task_t *)kmalloc(sizeof(task_t));
    // if (!new_thread_ptr) {
    //     uart_write_string("_new_thread: Unable to allocate new_thread_ptr!\n");
    //     return NULL;
    // }
    memset(new_thread_ptr, 0, sizeof(task_t));

    INIT_LIST_HEAD(&(new_thread_ptr->node));
    new_thread_ptr->state = TASK_RUNNING;
    new_thread_ptr->kernel_stack = (char *)alloc_pages(1);
    new_thread_ptr->kernel_stack_size = PAGE_SIZE;
    
    // new_thread_ptr->ppid = get_current_thread()->tgid;
    new_thread_ptr->user_stack = (char *)alloc_pages(1);
    new_thread_ptr->user_stack_size = PAGE_SIZE;
    new_thread_ptr->user_text = new_thread_ptr->text = NULL;

    new_thread_ptr->arg_loaded = 0;
    new_thread_ptr->argc = 0;
    
    my_bzero(new_thread_ptr->argv, sizeof(new_thread_ptr->argv));

    my_bzero(&(new_thread_ptr->old_reg_set), sizeof(new_thread_ptr->old_reg_set));
    new_thread_ptr->old_reg_set.lr = (uint64_t)real_thread;
    //stack is decremental
    new_thread_ptr->old_reg_set.sp = (uint64_t)STACK_BASE(new_thread_ptr->kernel_stack, new_thread_ptr->kernel_stack_size);
    new_thread_ptr->exit_code = new_thread_ptr->exit_state = new_thread_ptr->exit_signal = 0;
    new_thread_ptr->need_reschedule = 0;
    
    my_bzero(&(new_thread_ptr->reg_sig_handlers), sizeof(new_thread_ptr->reg_sig_handlers));
    INIT_LIST_HEAD(&(new_thread_ptr->pending_signal_list));

    uart_write_string("in _new_thread-2\n");

    // disable_interrupt();
    create_pgd(&new_thread_ptr->pgd);
    mappages(new_thread_ptr->pgd, 0x3C000000, 0x3000000, 0x3C000000);
    // test_enable_interrupt();

    uart_write_string("in _new_thread-3\n");
    // inherit from current thread.
    // task_t *current = get_current_thread();
    // new_thread_ptr->cwd = current->cwd;
    new_thread_ptr->cwd = rootfs->root;

    uart_write_string("in _new_thread-4\n");

    struct file *dummy;
    //stdin
    _vfs_open("/dev/uart", FILE_READ, &dummy, &(new_thread_ptr->open_files[0]));
    //stdout
    _vfs_open("/dev/uart", FILE_WRITE, &dummy, &(new_thread_ptr->open_files[1]));
    //stderr
    _vfs_open("/dev/uart", FILE_WRITE, &dummy, &(new_thread_ptr->open_files[2]));
    uart_write_string("in _new_thread-5\n");
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
    // stk_base = STACK_BASE(src->user_stack, src->user_stack_size);//src->user_stack + src->user_stack_size;
    // //user stack pointer is in the trap frame
    // stk_top = (char *)src_tf->sp;
    // offset = stk_base - stk_top;
    // memcpy(STACK_BASE(dst->user_stack, dst->user_stack_size) - offset, stk_top, offset);
    // //update user stack pointer
    // dst_tf->sp = (uint64_t)STACK_BASE(dst->user_stack, dst->user_stack_size) - offset;
    
    //setup user stack
    disable_interrupt();
    fork_pgd(src->pgd, dst->pgd);
    test_enable_interrupt();

    dst->exit_state = src->exit_state;
    dst->exit_code = src->exit_code;
    dst->exit_signal = src->exit_signal;

    //copy signal handlers
    memcpy(&(dst->reg_sig_handlers), &(src->reg_sig_handlers), sizeof(src->reg_sig_handlers));
    
    dst->old_reg_set.lr = return_from_fork;
    return dst;
}

task_t *copy_run_thread(task_t *src)
{
    task_t *dst = copy_thread(src);
    disable_interrupt();
    list_add_tail(&(dst->node), &running_queue);
    test_enable_interrupt();
    return dst;
}

task_t *create_thread_with_argc_argv(char *text, int argc, char **argv) {
    task_t *new_thread_ptr = new_thread();
    new_thread_ptr->text = text;
    //argument passing
    new_thread_ptr->argc = min(argc, 8);
    for (int i = 0; i < new_thread_ptr->argc && argv[i] != NULL; i++) {
        new_thread_ptr->argv[i] = argv[i];
    }
    disable_interrupt();
    list_add_tail(&(new_thread_ptr->node), &running_queue);
    test_enable_interrupt();
    return new_thread_ptr;
}

task_t *create_thread_with_argv(char *text, char **argv) {
    return create_thread_with_argc_argv(text, 8, argv);
}

task_t *create_thread(char *text)
{
    return create_thread_with_argc_argv(text, 0, NULL);
}

void destruct_thread(task_t *t)
{
    free_page(t->kernel_stack);
    free_page(t->user_stack);
    free_pgd(t->pgd);
    if (t->user_text) {
        free_page(t->user_text);
    }
    kfree(t);
}

void schedule()
{
    disable_interrupt();
    if (!list_empty(&running_queue)) {
        task_t *next_task = list_entry(running_queue.next, task_t, node);
        list_del_init(&(next_task->node));
        task_t *current = get_current_thread();
        if ((current->state | current->exit_state) == TASK_RUNNING)
            list_add_tail(&(current->node), &running_queue);
        // switch_to(&(current->old_reg_set), &(next_task->old_reg_set));
        test_enable_interrupt();
        // update_pgd(VA2PA(next_task->pgd));
        //context switch
        switch_to(&(current->old_reg_set), &(next_task->old_reg_set));
    } else {
        test_enable_interrupt();
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
    //free pending signals(a freeable resource for thread)
    struct signal *pend_sig, *safe;
    list_for_each_entry_safe(pend_sig, safe, &(current->pending_signal_list), node) {
        list_del(&(pend_sig->node));
        test_enable_interrupt();
        free_page(pend_sig->handler_user_stack);
        kfree(pend_sig);
        disable_interrupt();
    }
    list_del_init(current);
    list_add_tail(current, &stop_queue);
    test_enable_interrupt();
    schedule();
}

void kill_zombies()
{
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
        if (!list_empty(&stop_queue))
            kill_zombies();
        test_enable_interrupt();
        schedule();
    }
}

void one_shot_idle()
{
    disable_interrupt();
    if (!list_empty(&stop_queue))
        kill_zombies();
    test_enable_interrupt();
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

void init_startup_thread(char *main_addr)
{
    task_t *startup_thread_ptr = _new_thread();
    free_page(startup_thread_ptr->kernel_stack);
    startup_thread_ptr->kernel_stack_size = PAGE_SIZE;
    startup_thread_ptr->kernel_stack = KERN_BASE + LOW_MEMORY - PAGE_SIZE;
    startup_thread_ptr->text = main_addr;
    startup_thread_ptr->old_reg_set.lr = (uint64_t)main_addr;
    startup_thread_ptr->state = TASK_RUNNING;
    startup_thread_ptr->tid = next_free_tid();
    tid2task[startup_thread_ptr->tid] = startup_thread_ptr;
    write_sysreg(tpidr_el1, &(startup_thread_ptr->old_reg_set));
    // startup_thread_ptr->pgd = read_sysreg(ttbr0_el1);
    update_pgd(startup_thread_ptr->pgd);

    startup_thread_ptr->cwd = rootfs->root;
}

void demo_thread()
{
    for (int i = 0; i < 4; i++) {
        create_thread(thread_demo);
    }
}

void check_reschedule()
{
    task_t *current = get_current_thread();
    disable_interrupt();
    if (current->need_reschedule) {
        char *ret_addr = read_sysreg(elr_el1);
        asm volatile("mov x30, %0" : : "r" (ret_addr));
        current->need_reschedule = 0;
        test_enable_interrupt();
        schedule();
    } else {
        test_enable_interrupt();
    }
}

void time_reschedule(void *data)
{
    task_t *current = get_current_thread();
    current->need_reschedule = 1;
}

char *load_program(char *text, size_t file_size)
{
    task_t *current = get_current_thread();
    if (current->user_text != NULL) {
        free_page(current->user_text);
        current->user_text = NULL;
    }
    size_t page_needed = ALIGN(file_size, PAGE_SIZE) / PAGE_SIZE;
    char *load_addr = alloc_pages(page_needed);
    if (load_addr == NULL) {
        uart_write_string("No enough space for loading program.\n");
        return NULL;
    }
    memcpy(load_addr, text, file_size);
    current->user_text = load_addr;
    mappages(current->pgd, 0x0, file_size, VA2PA(load_addr));
    return 0;
    // task_t *current = get_current_thread();
    // if (current->user_text != NULL) {
    //     free_page(current->user_text);
    //     current->user_text = NULL;
    // }
    // size_t page_needed = ALIGN(file_size, PAGE_SIZE) / PAGE_SIZE;
    // char *load_addr = alloc_pages(page_needed);
    // if (load_addr == NULL) {
    //     uart_write_string("No enough space for loading program.\n");
    //     return NULL;
    // }
    // memcpy(load_addr, text, file_size);
    // return current->user_text = load_addr;
}

//return void to avoid setting x0
void check_load_args()
{
    task_t *current = get_current_thread();
    char **argv = current->argv;
    if (!current->arg_loaded) {
        current->arg_loaded = 1;
        asm volatile (
        "mov x0, %[arg1]\n\t"
        "mov x1, %[arg2]\n\t"
        "mov x2, %[arg3]\n\t"
        "mov x3, %[arg4]\n\t"
        "mov x4, %[arg5]\n\t"
        "mov x5, %[arg6]\n\t"
        "mov x6, %[arg7]\n\t"
        "mov x7, %[arg8]\n\t"
        "mov x8, %[arg9]\n\t"
        :
        : [arg1] "r" (argv[0]), [arg2] "r" (argv[1]), [arg3] "r" (argv[2]), [arg4] "r" (argv[3]),
          [arg5] "r" (argv[4]), [arg6] "r" (argv[5]), [arg7] "r" (argv[6]), [arg8] "r" (argv[7]), 
          [arg9] "r" (argv[8])
        );
    }
}

void check_before_switch_back()
{
    disable_interrupt();
    update_pgd(VA2PA(get_current_thread()->pgd));
    test_enable_interrupt();
    
    check_load_args();
    handle_current_signal();
    
    // asm volatile("b check_load_args\n\t");
    // asm volatile("b handle_current_signal\n\t");
}
