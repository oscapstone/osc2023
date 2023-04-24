#include "mini_uart.h"
#include "exception.h"
#include "syscall.h"
#include "mem_frame.h"
#include "thread.h"
#include "user_process.h"
#include "mailbox.h"
#include "peripherals/syscall.h"

int syscall_getpid(void)
{
        return get_pid();
}

unsigned int syscall_uartread(char buf[], unsigned int size)
{
        char c = '\0';
        for (int i = 0; i < size; i++) {
                c = uart_recv();
                if (c == '\r' || c == '\n') {
                        uart_endl();
                } else {
                        uart_send(c);
                }
                buf[i] = c;
        }
        return size;
}

unsigned int syscall_uartwrite(const char buf[], unsigned int size)
{
        int i;
        for (i = 0; i < size; i++) {
                if (buf[i] == '\0') break;
                if (buf[i] == '\r' || buf[i] == '\n') {
                        uart_endl();
                } else {
                        uart_send(buf[i]);
                }
        }
        return i;
}

int syscall_exec(char *name, char *const argv[])
{
        return replace_process(get_pid(), name, argv);
}

int syscall_fork(void)
{
        return fork_process(get_pid());
}

void syscall_exit(int status)
{
        exit_current_process();
}

int syscall_mbox_call(unsigned char ch, unsigned int *mbox)
{
        return mailbox_call(ch, mbox);
}

void syscall_kill(int pid)
{
        kill_process(pid);
}

void syscall_enterance(void)
{
        asm volatile("mov x15, x8");
        asm volatile("mov x14, x0");
        asm volatile("mov x13, x1");

        unsigned long syscall_num, arg0, arg1;
        unsigned long *sp_on_exception;
        asm volatile("mov %0, x15": "=r" (syscall_num));
        asm volatile("mov %0, x14": "=r" (arg0));
        asm volatile("mov %0, x13": "=r" (arg1));
        asm volatile("mov %0, x12" : "=r"(sp_on_exception));
        set_sp_on_exception((char*)sp_on_exception);

        int return_value = 0;
        switch (syscall_num)
        {
        case SYSCALL_NUM_GET_PID:
                return_value = syscall_getpid();
                break;
        case SYSCALL_NUM_UART_READ:
                return_value =
                        (int)syscall_uartread((char*)arg0, (unsigned int)arg1);
                break;
        case SYSCALL_NUM_UART_WRITE:
                return_value =
                        (int)syscall_uartwrite((char*)arg0, (unsigned int)arg1);
                break;
        case SYSCALL_NUM_EXEC:
                return_value = syscall_exec((char*)arg0, (char**)arg1);
                break;
        case SYSCALL_NUM_FORK:
                return_value = syscall_fork();
                break;
        case SYSCALL_NUM_EXIT:
                syscall_exit((int)arg0);
                break;
        case SYSCALL_NUM_MBOX_CALL:
                return_value = syscall_mbox_call((unsigned char)arg0,
                                                 (unsigned int*)arg1);
                break;
        case SYSCALL_NUM_KILL:
                syscall_kill((int)arg0);
                break;
        default:
                uart_send_string("[ERROR] invalid system call number\r\n");
        }

        unsigned long return_value_64 = 0;
        return_value_64 |= return_value;
        if (syscall_num != SYSCALL_NUM_EXIT
                && syscall_num != SYSCALL_NUM_KILL) {
                *sp_on_exception = return_value;
        }
}

/////////////////////////////////////////////////////////
//   DEMO                                              //
/////////////////////////////////////////////////////////

// TODO: complelte demo syscall or delete it
void demo_getpid(void)
{        
        uart_send_int(getpid());
        uart_endl();

        exit();
}

void demo_syscall(void)
{
        char *stack_alloc = allocate_frame(0);
        stack_alloc += FRAME_SIZE;
        branch_to_address_el0(demo_getpid, stack_alloc);
}