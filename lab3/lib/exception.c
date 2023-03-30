#include "muart.h"
#include "utils.h"
#include "timer.h"
#include "exception.h"

void enable_interrupt(void) {
    asm volatile("msr DAIFCLr, 0xf\r\n");
}

void disable_interrupt(void) {
    asm volatile("msr DAIFSet, 0xf\r\n");
}

void el0_irq_entry(void) {
    disable_interrupt();
    core_timer_handler();
    enable_interrupt();
}

void el1h_irq_entry(void) {
    disable_interrupt();

    if ((*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT) && (*CORE0_IRQ_SOURCE & IRQ_SOURCE_GPU)) {
        async_mini_uart_handler();
    } else if (*CORE0_IRQ_SOURCE & IRQ_SOURCE_CNTPNSIRQ) {
        reset_core_timer();
    }

    enable_interrupt();
}

void exception_entry(void) {
    disable_interrupt();

    unsigned long elr, esr, spsr;

    asm volatile("mrs %0, elr_el1"  :"=r"(elr)  ::"memory");
    asm volatile("mrs %0, esr_el1"  :"=r"(esr)  ::"memory");
    asm volatile("mrs %0, spsr_el1" :"=r"(spsr) ::"memory");

    mini_uart_puts("elr_el1:\t");
    printhex(elr);
    mini_uart_puts("\r\n");

    mini_uart_puts("esr_el1:\t");
    printhex(esr);
    mini_uart_puts("\r\n");

    mini_uart_puts("spsr_el1:\t");
    printhex(spsr);
    mini_uart_puts("\r\n\r\n");

    enable_interrupt();
}

void invalid_exception_entry(void) {
    disable_interrupt();
    mini_uart_puts("invalid exception!\r\n");
    enable_interrupt();
}