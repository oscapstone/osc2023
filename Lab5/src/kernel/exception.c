#include "peripherals/mini_uart.h"
#include "peripherals/irq.h"
#include "stdlib.h"
#include "timer.h"
#include "thread.h"
#include "syscall.h"

extern task_struct *get_current();
extern void enable_interrupt();
extern void disable_interrupt();
extern signal_handler signal_table[];

/**
 * common exception handler
 */
void exc_handler(unsigned long type, unsigned long esr, unsigned long elr, unsigned long spsr, unsigned long far)
{
    // print out interruption type
    switch (type)
    {
    case 0:
        uart_send_string("Synchronous");
        break;
    case 1:
        uart_send_string("IRQ");
        break;
    case 2:
        uart_send_string("FIQ");
        break;
    case 3:
        uart_send_string("SError");
        break;
    }
    uart_send_string(": ");
    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch (esr >> 26)
    {
    case 0b000000:
        uart_send_string("Unknown");
        break;
    case 0b000001:
        uart_send_string("Trapped WFI/WFE");
        break;
    case 0b001110:
        uart_send_string("Illegal execution");
        break;
    case 0b010101:
        uart_send_string("System call");
        break;
    case 0b100000:
        uart_send_string("Instruction abort, lower EL");
        break;
    case 0b100001:
        uart_send_string("Instruction abort, same EL");
        break;
    case 0b100010:
        uart_send_string("Instruction alignment fault");
        break;
    case 0b100100:
        uart_send_string("Data abort, lower EL");
        break;
    case 0b100101:
        uart_send_string("Data abort, same EL");
        break;
    case 0b100110:
        uart_send_string("Stack alignment fault");
        break;
    case 0b101100:
        uart_send_string("Floating point");
        break;
    default:
        uart_send_string("Unknown");
        break;
    }
    // decode data abort cause
    if (esr >> 26 == 0b100100 || esr >> 26 == 0b100101)
    {
        uart_send_string(", ");
        switch ((esr >> 2) & 0x3)
        {
        case 0:
            uart_send_string("Address size fault");
            break;
        case 1:
            uart_send_string("Translation fault");
            break;
        case 2:
            uart_send_string("Access flag fault");
            break;
        case 3:
            uart_send_string("Permission fault");
            break;
        }
        switch (esr & 0x3)
        {
        case 0:
            uart_send_string(" at level 0");
            break;
        case 1:
            uart_send_string(" at level 1");
            break;
        case 2:
            uart_send_string(" at level 2");
            break;
        case 3:
            uart_send_string(" at level 3");
            break;
        }
    }
    // dump registers
    uart_send_string("\nSPSR_EL1 ");
    uart_hex(spsr >> 32);
    uart_hex(spsr);
    uart_send_string(" ; ELR_EL1 ");
    uart_hex(elr >> 32);
    uart_hex(elr);
    uart_send_string(" ; ESR_EL1 ");
    uart_hex(esr >> 32);
    uart_hex(esr);
    uart_send_string(" ; FAR_EL1 ");
    uart_hex(far >> 32);
    uart_hex(far);
    uart_send_string("\n");

    return;
}

void el1_irq_interrupt_handler()
{
    unsigned int irq_basic_pending = get32(IRQ_BASIC_PENDING);
    irq_basic_pending &= (1 << 19); // clear bits

    // GPU IRQ 57 : UART Interrupt
    if (irq_basic_pending)
    {
        if (get32(AUX_MU_IIR_REG) & 0b100) // Receiver holds valid byte
        {
            uart_rx_handler();
        }
        else if (get32(AUX_MU_IIR_REG) & 0b010) // Transmit holding register empty
        {
            uart_tx_handler();
        }
    }
    // ARM Core Timer Interrupt
    else if (get32(CORE0_INTR_SRC) & (1 << 1))
    {
        long cntpct_el0, cntfrq_el0;
        asm volatile(
            "mrs %0, cntpct_el0;"
            "mrs %1, cntfrq_el0;"
            : "=r"(cntpct_el0), "=r"(cntfrq_el0));
        el1_timer_handler(cntpct_el0, cntfrq_el0);
    }

    return;
}

void el0_to_el1_sync_handler(unsigned long trapframe_addr)
{
    int syscall_no;
    trapframe *curr_trapframe = (trapframe *)trapframe_addr;
    asm volatile("mov %0, x8"
                 : "=r"(syscall_no)::);
    if (syscall_no == 0)
    {
        int pid = getpid();
        curr_trapframe->x[0] = pid;
    }
    else if (syscall_no == 1)
    {
        char *buf = (char *)curr_trapframe->x[0];
        unsigned int size = curr_trapframe->x[1];
        disable_uart_irq();
        enable_interrupt();
        unsigned int ret = uart_read(buf, size);
        curr_trapframe->x[0] = ret;
    }
    else if (syscall_no == 2)
    {
        char *buf = (char *)curr_trapframe->x[0];
        unsigned int size = curr_trapframe->x[1];
        unsigned int ret = uart_write(buf, size);
        curr_trapframe->x[0] = ret;
    }
    else if (syscall_no == 3)
    {
        char *name = (char *)curr_trapframe->x[0];
        char **argv = (char **)curr_trapframe->x[1];
        int ret = exec(name, argv);
        curr_trapframe->x[0] = ret;
    }
    else if (syscall_no == 4)
    {
        task_struct *current = get_current();
        current->status = FORKING;
        current->trapframe = trapframe_addr;
        int ret = fork();
        curr_trapframe = (trapframe *)get_current()->trapframe;
        curr_trapframe->x[0] = ret;
    }
    else if (syscall_no == 5)
    {
        int status = curr_trapframe->x[0];
        exit(status);
    }
    else if (syscall_no == 6)
    {
        unsigned char ch = (unsigned char)curr_trapframe->x[0];
        unsigned int *mbox_user = (unsigned int *)curr_trapframe->x[1];
        int ret = mbox_call_u(ch, mbox_user);
        curr_trapframe->x[0] = ret;
    }
    else if (syscall_no == 7)
    {
        int pid = (int)curr_trapframe->x[0];
        kill(pid);
    }
    else if (syscall_no == 8)
    {
        // signal
        int SIGNAL = (int)curr_trapframe->x[0];
        void (*handler)() = (void (*)())curr_trapframe->x[1];
        signal(SIGNAL, handler);
    }
    else if (syscall_no == 9)
    {
        // signal kill
        int pid = (int)curr_trapframe->x[0];
        int SIGNAL = (int)curr_trapframe->x[1];
        int if_cust = 0;
        task_struct *current = get_current();

        if (current->custom_signal)
        {
            custom_signal *cust_sig = current->custom_signal;
            do
            {
                if (cust_sig->sig_num == SIGNAL)
                {
                    if_cust = 1;
                    // signal's context save
                    sig_context_update(curr_trapframe, cust_sig->handler);
                    break;
                }
                cust_sig = container_of(cust_sig->list.next, custom_signal, list);
            } while (cust_sig != current->custom_signal);
        }
        else if (!current->custom_signal && !if_cust)
            (signal_table[SIGNAL])(pid);
    }
    else if (syscall_no == 10)
    {
        // signal restore
        sig_context_restore(curr_trapframe);

        disable_interrupt();
        task_struct *current = get_current();
        free(current->signal_context->trapframe);
        free(current->signal_context->user_stack);
        free(current->signal_context);
        current->signal_context = NULL;
        enable_interrupt();
    }
    else if (syscall_no == 11)
    {
        task_struct *current = get_current();
        if (current->thread_info->child_id == 0)
            printf("child here, fork() return value : %d\n", current->thread_info->child_id);
        else
            printf("parent here, fork() return value : %d\n", current->thread_info->child_id);
    }
}