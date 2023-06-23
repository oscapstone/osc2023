#include <mini_uart.h>
#include <utils.h>
#include <syscall.h>
#include <exec.h>
#include <task.h>
#include <current.h>
#include <cpio.h>
#include <dt17.h>
#include <preempt.h>
#include <kthread.h>
#include <mm.h>
#include <mbox.h>
#include <signal.h>
#include <mmu.h>
#include <vfs.h>

syscall_funcp syscall_table[] = {
    (syscall_funcp) syscall_getpid,     // 0
    (syscall_funcp) syscall_uart_read,  // 1
    (syscall_funcp) syscall_uart_write, // 2
    (syscall_funcp) syscall_exec,       // 3
    (syscall_funcp) syscall_fork,       // 4
    (syscall_funcp) syscall_exit,       // 5
    (syscall_funcp) syscall_mbox_call,  // 6
    (syscall_funcp) syscall_kill_pid,   // 7
    (syscall_funcp) syscall_signal,     // 8
    (syscall_funcp) syscall_kill,       // 9
    (syscall_funcp) syscall_mmap,       // 10
    (syscall_funcp) syscall_open,       // 11
    (syscall_funcp) syscall_close,      // 12
    (syscall_funcp) syscall_write,      // 13
    (syscall_funcp) syscall_read,       // 14
    (syscall_funcp) syscall_mkdir,      // 15
    (syscall_funcp) syscall_mount,      // 16
    (syscall_funcp) syscall_chdir,      // 17
    (syscall_funcp) syscall_lseek64,    // 18
    (syscall_funcp) syscall_ioctl,      // 19
    (syscall_funcp) syscall_sigreturn,  // 20
    (syscall_funcp) syscall_test,       // 21
};

static inline void copy_regs(struct pt_regs *regs)
{
    regs->x19 = current->regs.x19;
    regs->x20 = current->regs.x20;
    regs->x21 = current->regs.x21;
    regs->x22 = current->regs.x22;
    regs->x23 = current->regs.x23;
    regs->x24 = current->regs.x24;
    regs->x25 = current->regs.x25;
    regs->x26 = current->regs.x26;
    regs->x27 = current->regs.x27;
    regs->x28 = current->regs.x28;
}

void syscall_handler(trapframe *regs)
{
    uint64 syscall_num;

    syscall_num = regs->x8;

    if(syscall_num > 2)
        // uart_sync_printf("syscall number:%d\n", syscall_num);

    if (syscall_num >= ARRAY_SIZE(syscall_table)) {
        // Invalid syscall
        return;
    }

    enable_interrupt();
    // TODO: bring the arguments to syscall
    (syscall_table[syscall_num])(
        regs,
        regs->x0,
        regs->x1,
        regs->x2,
        regs->x3,
        regs->x4,
        regs->x5
    );
    disable_interrupt();
}

void syscall_getpid(trapframe *frame){
    
    frame->x0 = current->tid;
}

void syscall_uart_read(trapframe *_, char buf[], size_t size){
    uart_recvn(buf, size);
}

void syscall_uart_write(trapframe *_, const char buf[], size_t size){
    uart_sendn(buf, size);
}

void syscall_exec(trapframe *_, const char* name, char *const argv[]){
    struct file f;
    void *data;
    char *kernel_sp;
    int datalen, adj_datalen, ret;

    ret = vfs_open(name, 0, &f);
    if(ret < 0)
        return;
    datalen = f.vnode->v_ops->getsize(f.vnode);
    if(datalen < 0)
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
    kernel_sp = (char *)current->kernel_stack + STACK_SIZE - 0x10;

    // Reset signal
    signal_head_reset(current->signal);
    sighand_reset(current->sighand);
    //Rest addr space & page table
    task_reset_mm(current);
    task_init_map(current);

    // 0x000000000000 ~ <datalen>: rwx: Code
    vma_map(current->address_space, (void *)0, adj_datalen, VMA_R | VMA_W | VMA_X | VMA_KVA, data);

    set_page_table(current->page_table);
    exec_user_prog((void *)0, (char *)0xffffffffeff0, kernel_sp);
}

void syscall_fork(trapframe *frame){
    task_struct *child;
    trapframe *child_frame;

    child = task_create();

    child->kernel_stack = kmalloc(STACK_SIZE);

    memncpy(child->kernel_stack, current->kernel_stack, STACK_SIZE);

    // TODO: Implement copy on write

    vma_meta_copy(child->address_space, current->address_space, current->page_table);

    // Copy the signal handler
    sighand_copy(child->sighand);

    // Save regs
    SAVE_REGS(current);

    // Copy register
    copy_regs(&child->regs);

    // Copy stack realted registers
    child->regs.fp = KSTACK_VARIABLE(current->regs.fp);
    child->regs.sp = KSTACK_VARIABLE(current->regs.sp);

    // https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
    child->regs.lr = &&SYSCALL_FORK_END;

    // Adjust child trapframe
    child_frame = KSTACK_VARIABLE(frame);

    // return value of fork for child process is 0
    child_frame->x0 = 0;
    // child_frame->x30 = (uint64)DATA_VARIABLE(frame->x30);
    // child_frame->sp_el0 = USTACK_VARIABLE(frame->sp_el0);
    // child_frame->elr_el1 = DATA_VARIABLE(frame->elr_el1);

    sched_add_task(child);

    // Set return value of parent process
    frame->x0 = child->tid;

SYSCALL_FORK_END:

    asm volatile("nop");

}

void syscall_exit(trapframe *_)
{
    exit_user_prog();
}

void syscall_mbox_call(trapframe *_, unsigned char ch, unsigned int *mbox)
{
    // copy data to kernel since mbox can't access the user space memory
    int mbox_size = (int)mbox[0];
    if(mbox_size <= 0)
        return;
    char *kmbox = kmalloc(mbox_size);
    memncpy(kmbox, (char *)mbox, mbox_size);
    mbox_call(ch, (unsigned int*)kmbox);
    memncpy((char *)mbox, kmbox, mbox_size);
    kfree(kmbox);
}

void syscall_kill_pid(trapframe *_, int pid)
{
    task_struct *task;
    if(current->tid == pid){
        exit_user_prog();

        return;
    }

    preempt_disable();

    task = task_get_by_tid(pid);

    if(!task || task->status != TASK_RUNNING){
        goto SYSCALL_KILL_PID_END;
    }

    list_del(&task->list);
    kthread_add_wait_queue(task);

SYSCALL_KILL_PID_END:
    preempt_enable();
}

void syscall_test(trapframe *_)
{
    uint64 spsr_el1;
    uint64 elr_el1;
    uint64 esr_el1;

    spsr_el1 = read_sysreg(spsr_el1);
    elr_el1 = read_sysreg(elr_el1);
    esr_el1 = read_sysreg(esr_el1);

    uart_printf("[TEST] spsr_el1: %llx; elr_el1: %llx; esr_el1: %llx\r\n", 
        spsr_el1, elr_el1, esr_el1);

}

void exit_to_user_mode(trapframe regs)
{
    enable_interrupt();
    handle_signal(&regs);
    disable_interrupt();
}