#include <mini_uart.h>
#include <utils.h>
#include <syscall.h>
#include <exec.h>

syscall_funcp syscall_table[] = {
    (syscall_funcp) syscall_getpid,     // 0
    (syscall_funcp) syscall_uartread,   // 1
    (syscall_funcp) syscall_uartwrite,  // 2
    (syscall_funcp) syscall_exec,       // 3
    (syscall_funcp) syscall_fork,       // 4
    (syscall_funcp) syscall_exit,       // 5
    (syscall_funcp) syscall_mbox_call,  // 6
    (syscall_funcp) syscall_kill_pid,   // 7
    (syscall_funcp) syscall_signal,     // 8
    (syscall_funcp) syscall_kill,       // 9
    (syscall_funcp) syscall_test,  // 10
};

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
        regs.x0,
        regs.x1,
        regs.x2,
        regs.x3,
        regs.x4,
        regs.x5
    );
    disable_interrupt();
}

void *syscall_getpid(void){
    // TODO
    return NULL;
}

void *syscall_uartread(char buf[], size_t size)
{
    // TODO
    return NULL;
}

void *syscall_uartwrite(const char buf[], size_t size)
{
    // TODO
    return NULL;
}

void *syscall_exec(const char* name, char *const argv[])
{
    // TODO
    return NULL;
}

void *syscall_fork(void)
{
    // TODO
    return NULL;
}

void *syscall_exit(void)
{
    exit_user_prog();

    return NULL;
}

void *syscall_mbox_call(unsigned char ch, unsigned int *mbox)
{
    // TODO
    return NULL;
}

void *syscall_kill_pid(int pid)
{
    // TODO
    return NULL;
}

void *syscall_signal(int signal, void (*handler)(void))
{
    // TODO
    return NULL;
}

void *syscall_kill(int pid, int signal)
{
    // TODO
    return NULL;
}

void *syscall_test()
{
    uint64 spsr_el1;
    uint64 elr_el1;
    uint64 esr_el1;

    spsr_el1 = read_sysreg(spsr_el1);
    elr_el1 = read_sysreg(elr_el1);
    esr_el1 = read_sysreg(esr_el1);

    uart_printf("[TEST] spsr_el1: %llx; elr_el1: %llx; esr_el1: %llx\r\n", 
        spsr_el1, elr_el1, esr_el1);

    return NULL;
}