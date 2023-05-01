#include "syscall.h"
#include "thread.h"
#include "uart.h"
#include "utils.h"
#include "initramfs.h"
#include "mm.h"
#include "mailbox.h"

void syscall_getpid(struct trap_frame *tf)
{
    task_t *current = get_current_thread();
    tf->gprs[0] = current->tid;
}
void syscall_uart_read(struct trap_frame *tf)
{
    char *buf = tf->gprs[0];
    size_t size = tf->gprs[1];
    size_t i;
    for (i = 0; i < size; i++)
        kuart_write(buf[i]);
    tf->gprs[0] = i;
}
void syscall_uart_write(struct trap_frame *tf)
{
    char *buf = tf->gprs[0];
    size_t size = tf->gprs[1];
    size_t i;
    for (i = 0; i < size; i++)
        buf[i] = kuart_read();
    tf->gprs[0] = i;
}

void run_user_prog(char *user_text)
{
    task_t *current = get_current_thread();
    //write lr
    __asm__ __volatile__("mov x30, %[value]"
                         :
                         : [value] "r" (exit)
                         : "x30");
    //write fp
    __asm__ __volatile__("mov x29, %[value]"
                         :
                         : [value] "r" (STACK_BASE(current->user_stack, current->user_stack_size))
                         : "x29");
    //allow interrupt
    write_sysreg(spsr_el1, 0);
    write_sysreg(elr_el1, user_text);
    write_sysreg(sp_el0, STACK_BASE(current->user_stack, current->user_stack_size));
    asm volatile("eret");
}

//Run the program with parameters.
//The exec() functions return only if an error has occurred.
//The return value is -1
void syscall_exec(struct trap_frame *tf)
{
    const char *name = tf->gprs[0];
    char **const argv = tf->gprs[1];
    //reform all syscall_function into proper form
    size_t file_size;
    char *content = _initramfs.file_content(&_initramfs, name, &file_size);
    if (content == NULL) {
        //file not found or not a valid new ascii format cpio archive file
        tf->gprs[0] = -1;
    } else {
        run_user_prog(content);
        //never go here
        tf->gprs[0] = -1;
    }
}
void syscall_fork(struct trap_frame *tf)
{
    //if is parent thread, retrurn child thread id
    //otherwise, return 0
    task_t *current = get_current_thread();
    task_t *child = copy_run_thread(current);
    tf->gprs[0] = child->tid;
}
void syscall_exit(struct trap_frame *tf)
{
    _exit(0);
}
void syscall_mbox_call(struct trap_frame *tf)
{
    unsigned char ch = tf->gprs[0];
    unsigned int *mbox = tf->gprs[1];
    mailbox_call(mbox, ch);
    tf->gprs[0] = mbox[1];
}
void syscall_kill(struct trap_frame *tf)
{
    int pid = tf->gprs[0];
    //TODO
    //You don’t need to implement this system call if you prefer to kill a process using the POSIX Signal stated in Advanced Exercise 1.
    task_t *t = tid2task[pid];
    disable_interrupt();
    if (t->exit_state | t->state == TASK_RUNNING) {
        list_del(t);
        list_add_tail(t, &stop_queue);
    }
    test_enable_interrupt();
    t->exit_code = 9;
    t->state = 0;
    t->exit_state = EXIT_ZOMBIE;
    // schedule();
}

void syscall_signal(struct trap_frame *tf)
{
    
}

void syscall_sigkill(struct trap_frame *tf)
{
    
}
void syscall_sigreturn(struct trap_frame *tf)
{
    
}

syscall_t default_syscall_table[NUM_syscalls] = {
    [SYS_GETPID] = &syscall_getpid,
    [SYS_UART_RECV] = &syscall_uart_read,
    [SYS_UART_WRITE] = &syscall_uart_write,
    [SYS_EXEC] = &syscall_exec,
    [SYS_FORK] = &syscall_fork,
    [SYS_EXIT] = &syscall_exit,
    [SYS_MBOX] = &syscall_mbox_call,
    [SYS_KILL] = &syscall_kill,
    [SYS_SIGNAL] = &syscall_signal,
    [SYS_SIGKILL] = &syscall_sigkill,
    [SYS_SIGRETURN] = &syscall_sigreturn,
};

void fork_test(){
    // printf("\nFork Test, pid %d\n", getpid());
    uart_write_string("\nFork Test, pid ");
    uart_write_no(getpid());
    uart_write_string("\n");;
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        uart_write_string("first child pid: ");
        uart_write_no(getpid());
        uart_write_string(", cnt: ");
        uart_write_no(cnt);
        uart_write_string(", ptr: 0x");
        uart_write_no_hex(&cnt);
        uart_write_string(", sp : 0x");
        uart_write_no_hex(cur_sp);
        uart_write_string("\n");
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
            uart_write_string("first child pid: ");
            uart_write_no(getpid());
            uart_write_string(", cnt: ");
            uart_write_no(cnt);
            uart_write_string(", ptr: 0x");
            uart_write_no_hex(&cnt);
            uart_write_string(", sp : 0x");
            uart_write_no_hex(cur_sp);
            uart_write_string("\n");
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                // printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                uart_write_string("second child pid: ");
                uart_write_no(getpid());
                uart_write_string(", cnt: ");
                uart_write_no(cnt);
                uart_write_string(", ptr: 0x");
                uart_write_no_hex(&cnt);
                uart_write_string(", sp : 0x");
                uart_write_no_hex(cur_sp);
                uart_write_string("\n");
                delay(1000000);
                ++cnt;
            }
        }
        exit();
    }
    else {
        // printf("parent here, pid %d, child %d\n", getpid(), ret);
        uart_write_string("parent here, pid ");
        uart_write_no(getpid());
        uart_write_string(", child ");
        uart_write_no(ret);
        uart_write_string("\n");
    }
}