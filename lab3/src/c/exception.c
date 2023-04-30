#include "printf.h"
#include "timer.h"
#include "uart.h"

#define CORE0_IRQ_SOURCE ((volatile unsigned int *)0x40000060)
#define IRQ_PEND_1 ((volatile unsigned int *)0x3f00B204)
#define IRQS1 (volatile unsigned int *)0x3f00b210

extern struct queue uart_write, uart_read;

void svc_router(unsigned long spsr, unsigned long elr, unsigned long esr)
{
    unsigned int svc_n = esr & 0xFFFFFF;

    switch (svc_n)
    {
    case 0:
        printf("\n");
        printf("spsr_el1\t%x\n", spsr);
        printf("elr_el1\t\t%x\n", elr);
        printf("esr_el1\t\t%x\n", esr);
        printf("\n");

        break;

    case 1:
        printf("enable core timer\n");
        core_timer_enable();
        break;

    case 2:
        printf("disable core timer\n");
        core_timer_disable();
        break;

    case 3:
        set_new_timeout();
        break;

    case 4:
        asm volatile(
            "ldr x0, =0x345             \n\t"
            "msr spsr_el1, x0           \n\t"
            "ldr x0, = shell_start      \n\t"
            "msr elr_el1,x0             \n\t"
            "eret                       \n\t");

    default:
        break;
    }

    return;
}

void print_invalid_entry_message()
{
    printf("invalid exception!\n");
    return;
}

void disable_irq()
{
    asm volatile("msr DAIFSet, 0xf");
}
void enable_irq()
{
    asm volatile("msr DAIFClr, 0xf");
}

void mini_uart_interrupt_enable()
{
    *IRQS1 |= (1 << 29);
    *AUX_MU_IER = 0x1;
    queue_init(&uart_read, 1024);
    queue_init(&uart_write, 1024);
}

void irq_router()
{
    unsigned int irq = *CORE0_IRQ_SOURCE;
    unsigned int irq1 = *IRQ_PEND_1;

    if (irq & 0x2)
    {
        timer_router();
    }
    else if (irq1 & (1 << 29))
    {
        mini_uart_handler();
    }
}