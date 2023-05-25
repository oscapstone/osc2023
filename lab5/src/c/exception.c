#include "printf.h"
#include "timer.h"
#include "exception.h"
#include "syscall.h"

void svc_router(unsigned long spsr, unsigned long elr, unsigned long esr, struct trapframe *tf);
void syscall_router(int sys_call_n, struct trapframe *tf);
void print_invalid_entry_message();

void svc_router(unsigned long spsr, unsigned long elr, unsigned long esr, struct trapframe *tf)
{
    if (esr >> 26 == 0b010101) //  is the value of SVC instruction
    {
        unsigned int svc_n = esr & 0xFFFF;

        switch (svc_n)
        {
        case 2:
            printf("\n");
            printf("spsr_el1\t%x\n", spsr);
            printf("elr_el1\t\t%x\n", elr);
            printf("esr_el1\t\t%x\n", esr);
            printf("\n");

            break;

        case 1:
            asm volatile(
                "ldr x0, =0x345             \n\t"
                "msr spsr_el1, x0           \n\t"
                "ldr x0, = shell_start      \n\t"
                "msr elr_el1,x0             \n\t"
                "eret                       \n\t");

        case 0:
            syscall_router(tf->x[8], tf);
            break;

        default:
            break;
        }
    }
    else
    {
        printf("Exception return address 0x%x\n", elr);
        printf("Exception class (EC) 0x%x\n", esr >> 26);
    }

    return;
}

void syscall_router(int sys_call_n, struct trapframe *tf)
{
    switch (sys_call_n)
    {
    case SYS_GETPID:
        sys_getpid(tf);
        break;
    case SYS_UART_READ:
        sys_uart_read(tf);
        break;
    case SYS_UART_WRITE:
        sys_uart_write(tf);
        break;
    case SYS_EXEC:
        sys_exec(tf);
        break;
    case SYS_EXIT:
        sys_exit();
        break;
    case SYS_FORK:
        sys_fork(tf);
        break;
    case SYS_MBOX_CALL:
        sys_mbox_call(tf);
        break;
    case SYS_KILL:
        sys_kill(tf);
        break;
    case SYS_SIGNAL:
        sys_signal(tf);
        break;
    case SYS_POSIX_KILL:
        sys_posix_kill(tf);
        break;
    case SYS_SCHEDULE:
        sys_schedule();
        break;
    case SYS_ENABLE_CORE_TIMER:
        sys_enable_core_timer();
        printf("enable core timer\n");
        break;
    case SYS_DISABLE_CORE_TIMER:
        sys_disable_core_timer();
        printf("disable core timer\n");
        break;
    case SYS_SET_TIMEOUT:
        sys_set_timeout(tf);
        break;
    default:
        break;
    }
}

void print_invalid_entry_message()
{
    printf("invalid exception!\n");
    return;
}
