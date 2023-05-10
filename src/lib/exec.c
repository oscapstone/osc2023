#include <type.h>
#include <current.h>
#include <sched.h>
#include <task.h>

#define STACK_SIZE ( 2 * PAGE_SIZE)

void enter_el0_run_user_prog(void *entry, void *user_sp);

static void user_prog_start(void){
    char *user_sp;

    user_sp = (char *)current->user_stack + STACK_SIZE - 0x10;

    enter_el0_run_user_prog(user_sp, current->data);

    // User program should call exit() to terminate
}

static inline void pt_regs_init(struct pt_regs *regs){
    regs->x19 = 0;
    regs->x20 = 0;
    regs->x21 = 0;
    regs->x22 = 0;
    regs->x23 = 0;
    regs->x24 = 0;
    regs->x25 = 0;
    regs->x26 = 0;
    regs->x27 = 0;
    regs->x28 = 0;
    regs->fp = 0;
    regs->lr = user_prog_start;
}

void exec_user_prog(char* cpio, char *filename){
    task_struct *task;
    task = task_create();

    task->kernel_stack = kmalloc(STACK_SIZE);
    task->user_stack = kmalloc(STACK_SIZE);
    task->data = cpio_load_prog(cpio, filename);

    if(task->data == NULL)
        goto EXEC_USER_PROG_END;

    task->regs.sp = (char *)task->kernel_stack + STACK_SIZE -0x10;
    pt_regs_init(&task->regs);

    sched_add_task(task);

    return;

EXEC_USER_PROG_END:
    task_free(task);
}


void exit_user_prog(void){
    kthread_fini();
}