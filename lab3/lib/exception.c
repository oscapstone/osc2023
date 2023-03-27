#include "muart.h"
#include "utils.h"
#include "timer.h"
#include "exception.h"

void el0_irq_entry(void) {
    mini_uart_puts("received core timer interrupt!\r\n");

    unsigned long current   = get_current_time();
    unsigned long frequency = get_core_frequency();

    mini_uart_puts("seconds:\t");
    printdec(current / frequency);
    mini_uart_puts("\r\n");

    mini_uart_puts("frequency:\t");
    printdec(frequency);
    mini_uart_puts("\r\n\r\n");

    core_timer_handler();
}

void el1h_irq_entry(void) {
    mini_uart_puts("el1h_irq_entry\r\n");
}

void exception_entry(void) {
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
}

void invalid_exception_entry(void) {
    mini_uart_puts("invalid exception!\r\n");
}