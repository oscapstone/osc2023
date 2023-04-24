#include "uart.h"
#include "timer.h"
#include "task.h"
#include "interrupt.h"


void enable_interrupt() {
    asm volatile("msr DAIFClr, 0xf");
}

void disable_interrupt() {
    asm volatile("msr DAIFSet, 0xf");
}

void irq_handler(unsigned long long x0) {
    if (*CORE0_INT_SRC & CORE0_INT_SRC_GPU && *IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT) { // uart interrupt: gpu, aux
        if (*AUX_MU_IIR & (0b01 << 1)) {
            // Transmit holding register empty
            disable_uart_tx_interrupt();
            add_task(uart_tx_interrupt_handler, UART_IRQ_PRIORITY);
            pop_task();
        }
        else if (*AUX_MU_IIR & (0b10 << 1)) {
            // Recevier holding valid byte
            disable_uart_rx_interrupt();
            add_task(uart_rx_interrupt_handler, UART_IRQ_PRIORITY);
            pop_task();
        }
        else {
            uart_printf("uart interrupt error\n");
        }
    }
    else if (*CORE0_INT_SRC & CORE0_INT_SRC_TIMER) { // timer interrupt
        core_timer_interrupt_disable();
        add_task(core_timer_handler, TIMER_IRQ_PRIORITY);
        pop_task();
        core_timer_interrupt_enable();
    }
}

void invalid_exception_handler(unsigned long long x0) {
    uart_printf("invalid handler 0x%x\n", x0);
}

void sync_el0_64_handler() {
    unsigned long long spsr, elr, esr;
    asm volatile(
        "mrs %0, spsr_el1\n\t":"=r"(spsr):
    );
    asm volatile(
        "mrs %0, elr_el1\n\t":"=r"(elr):
    );
    asm volatile(
        "mrs %0, esr_el1 \n\t":"=r"(esr):
    );
    uart_printf("spsr_el1: %d, elr_el1: %d, esr_el1: %d\n", spsr, elr, esr);
}


void set_cpacr_el1(){
    asm volatile(
        "mov x1, (3 << 20)\n\t"
        "msr CPACR_EL1, x1\n\t"
    );
}

