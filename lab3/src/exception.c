#include "mini_uart.h"
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
    print_boottime();
}

void el1h_irq_entry(void) {
    disable_interrupt();
    

    if ((*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT) && (*CORE0_IRQ_SOURCE & IRQ_SOURCE_GPU)) { //IRQ_SOURCE_GPU => One or more bits set in pending register 1
        //ensure ( interrupt and aux )  and ( gpu interrupt )  // bit [8][9] meansThere are some interrupts pending which you don't know about. They are in pending register 1 /2." 
        async_uart_handler();
    } 
    else if (*CORE0_IRQ_SOURCE & IRQ_SOURCE_CNTPNSIRQ) {
        core_timer_handler();
    }

    enable_interrupt();

}

void exception_entry(void) {

    disable_interrupt();


    unsigned int spsr_el1, elr_el1, esr_el1;

    asm volatile("mrs %0, spsr_el1" :"=r"(spsr_el1));
    asm volatile("mrs %0, elr_el1"  :"=r"(elr_el1));
    asm volatile("mrs %0, esr_el1"  :"=r"(esr_el1));

    uart_puts("spsr_el1:\t");
    uart_hex(spsr_el1);
    uart_puts("\r\n");

    uart_puts("elr_el1:\t");
    uart_hex(elr_el1);
    uart_puts("\r\n");

    uart_puts("esr_el1:\t");
    uart_hex(esr_el1);
    uart_puts("\r\n");

    uart_puts("\r\n");

    enable_interrupt();
    
    return;
}

void invalid_exception_entry(void) {
    uart_puts("Undefined exception called !\r\n");
}