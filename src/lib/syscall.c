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
    (syscall_funcp) syscall_test,       // 10
    (syscall_funcp) syscall_sigreturn,  // 11
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

void syscall_handler(trapframe regs, uint32 syn)
{
    esr_el1 *esr = (esr_el1 *)&syn;
    uint64 syscall_num;

    // SVC instruction execution
    if(esr->ec != 0x15)
        return;

    syscall_num = regs.x8;

    if (syscall_num >= ARRAY_SIZE(syscall_table)) {
        // Invalid syscall
        return;
    }

    enable_interrupt();
    // TODO: bring the arguments to syscall
    (syscall_table[syscall_num])(
        &regs,
        regs.x0,
        regs.x1,
        regs.x2,
        regs.x3,
        regs.x4,
        regs.x5
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
    void *data;
    char *kernel_sp;
    char *user_sp;
    uint32 datalen;

    datalen = cpio_load_prog((char *)_initramfs_addr, name, (char **)&data);

    if(datalen == 0)
        return;

    kfree(current->data);
    current->data = data;
    current->datalen = datalen;

    kernel_sp = (char *)current->kernel_stack + STACK_SIZE - 0x10;
    user_sp = (char *)current->user_stack + STACK_SIZE - 0x10;

    // Reset signal
    signal_head_reset(current->signal);
    sighand_reset(current->sighand);
    exec_user_prog(current->data, user_sp, kernel_sp);
}

void syscall_fork(trapframe *frame){
    task_struct *child;
    trapframe *child_frame;

    child = task_create();

    child->kernel_stack = kmalloc(STACK_SIZE);
    child->user_stack = kmalloc(STACK_SIZE);
    child->data = kmalloc(current->datalen);
    child->datalen = current->datalen;

    memncpy(child->kernel_stack, current->kernel_stack, STACK_SIZE);
    memncpy(child->user_stack, current->user_stack, STACK_SIZE);
    memncpy(child->data, current->data, current->datalen);

    // Copy the signal handler
    sighand_copy(child->sighand, child->data);

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

    child_frame->x0 = 0;
    child_frame->x30 = (uint64)DATA_VARIABLE(frame->x30);
    child_frame->sp_el0 = USTACK_VARIABLE(frame->sp_el0);
    child_frame->elr_el1 = DATA_VARIABLE(frame->elr_el1);

    sched_add_task(child);

    // Set return value
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
    mbox_call(ch, mbox);
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