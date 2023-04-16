#include <mini_uart.h>
#include <utils.h>
#include <syscall.h>

funcp syscall_table[] = {
    syscall_test, // 0
    syscall_exit, // 1
};

void syscall_handler(uint32 syn)
{
    esr_el1 *esr = (esr_el1 *)&syn;

    int syscall_num = esr->iss & 0xffff;

    if (syscall_num >= ARRAY_SIZE(syscall_table)) {
        // Invalid syscall
        return;
    }

    // TODO: bring the arguments to syscall
    (syscall_table[syscall_num])();
}

void *syscall_exit()
{
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