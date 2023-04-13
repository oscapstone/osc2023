#include "uart.h"
#include "exception.h"
#include "timer.h"
#include "task.h"

void sync_el0_64_handler(unsigned long long x0)
{
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long esr_el1;

    asm volatile("mrs %0, SPSR_EL1\n"
                 : "=r"(spsr_el1)
                 :
                 : "memory");
    asm volatile("mrs %0, ELR_EL1\n"
                 : "=r"(elr_el1)
                 :
                 : "memory");
    asm volatile("mrs %0, ESR_EL1\n"
                 : "=r"(esr_el1)
                 :
                 : "memory");

    uart_async_printf("+-------------------------------+\n");
    uart_async_printf("| SPSR_EL1:\t0x%x\t|\n| ESR_EL1:\t0x%x\t|\n| ELR_EL1:\t0x%x\t|\n", spsr_el1, esr_el1, elr_el1);
    uart_async_printf("+-------------------------------+\n");
}

void irq_handler(unsigned long long x0)
{
    if (*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception
    {
        if (*AUX_MU_IIR & (0b01 << 1)) // TX can get data from write buffer
        {
            disable_mini_uart_tx_interrupt();
            add_task(uart_tx_interrupt_handler, UART_IRQ_PRIORITY);
            pop_task();
        }
        else if (*AUX_MU_IIR & (0b10 << 1))  // kernel can get data from read buffer
        {
            disable_mini_uart_rx_interrupt();
            add_task(uart_rx_interrupt_handler, UART_IRQ_PRIORITY);
            pop_task();
        }
        else
            uart_printf("uart handler error\n");
    }
    else if (*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)
    {
        core_timer_interrupt_disable();
        add_task(core_timer_handler, TIMER_IRQ_PRIORITY);
        pop_task();
        core_timer_interrupt_enable();
    }
}

void invalid_exception_handler(unsigned long long x0)
{
    uart_printf("invalid exception : 0x%x\n", x0);
    uart_getc();
}

void enable_interrupt()
{
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt()
{
    asm volatile("msr daifset, 0xf");
}