#include <type.h>
#include <current.h>
#include <sched.h>
#include <task.h>
#include <cpio.h>
#include <mm.h>
#include <kthread.h>

void enter_el0_run_user_prog(void *entry, void *user_sp);

void user_prog_start(void){
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

void sched_new_user_prog(char* cpio, char *filename){
    task_struct *task;
    void *data;
    uint32 datalen;

    datalen = cpio_load_prog(cpio, filename, (char **)&data);

    if(datalen == 0)
        goto EXEC_USER_PROG_END;

    task = task_create();

    task->kernel_stack = kmalloc(STACK_SIZE);
    task->user_stack = kmalloc(STACK_SIZE);
    task->data = data;
    task->datalen = datalen;

    task->regs.sp = (char *)task->kernel_stack + STACK_SIZE -0x10;
    pt_regs_init(&task->regs);

    sched_add_task(task);

EXEC_USER_PROG_END:
    return;
}


void exit_user_prog(void){
    kthread_fini();
}