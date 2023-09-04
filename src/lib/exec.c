#include <type.h>
#include <current.h>
#include <sched.h>
#include <task.h>
#include <cpio.h>
#include <mm.h>
#include <kthread.h>
#include <dt17.h>
#include <vfs.h>

void enter_el0_run_user_prog(void *entry, void *user_sp);

void user_prog_start(void){
    // char *user_sp;

    // user_sp = (char *)current->user_stack + STACK_SIZE - 0x10;

    enter_el0_run_user_prog((void *)0, (char*)0xffffffffeff0);

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

void sched_new_user_prog(char *pathname){
    task_struct *task;
    struct file f;
    void *data;
    int datalen, adj_datalen, ret;

    ret = vfs_open(pathname, 0, &f);
    if(ret < 0)
        return;
    
    datalen = f.vnode->v_ops->getsize(f.vnode);
    if(datalen<0)
        return;
    adj_datalen = ALIGN(datalen, PAGE_SIZE);
    data = kmalloc(adj_datalen);
    memzero(data, adj_datalen);
    ret = vfs_read(&f, data, datalen);
    if(ret < 0){
        kfree(data);
        return;
    }
    vfs_close(&f);
    task = task_create();
    task->kernel_stack = kmalloc(STACK_SIZE);
    task->regs.sp = (char *)task->kernel_stack + STACK_SIZE -0x10;
    pt_regs_init(&task->regs);
    task_init_map(task);
    
    // 0x000000000000 ~ <datalen>: rwx: Code
    vma_map(task->address_space, (void *)0, adj_datalen, VMA_R | VMA_W | VMA_X | VMA_KVA, data);

    sched_add_task(task);
}


void exit_user_prog(void){
    kthread_fini();
}