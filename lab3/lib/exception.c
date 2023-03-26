#include "muart.h"
#include "utils.h"
#include "exception.h"

void exception_entry(void) {
    unsigned long elr, esr, spsr;

    asm volatile("mrs %0, elr_el1"  :"=r"(elr));
    asm volatile("mrs %0, esr_el1"  :"=r"(esr));
    asm volatile("mrs %0, spsr_el1" :"=r"(spsr));

    mini_uart_puts("elr_el1:\t");
    printhex(elr);
    mini_uart_puts("\r\n");

    mini_uart_puts("esr_el1:\t");
    printhex(esr);
    mini_uart_puts("\r\n");

    mini_uart_puts("spsr_el1:\t");
    printhex(spsr);
    mini_uart_puts("\r\n");

    while (1);
}