#include "uart.h"
#include "timer.h"
#include "task.h"
#include "interrupt.h"

char tx_buffer[MAX_BUF_SIZE];
char rx_buffer[MAX_BUF_SIZE];
unsigned int tx_bf_r_idx = 0;
unsigned int tx_bf_w_idx = 0;
unsigned int rx_bf_r_idx = 0;
unsigned int rx_bf_w_idx = 0;

void enable_uart_interrupt() {
    enable_uart_tx_interrupt();
    enable_uart_rx_interrupt();
    *ENABLE_IRQS_1 |= 1 << 29;
}

void enable_uart_tx_interrupt() {
    *AUX_MU_IER |= 2;
}

void disable_uart_tx_interrupt() {
    *AUX_MU_IER &= ~(2);
}

void enable_uart_rx_interrupt() {
    *AUX_MU_IER |= 1;
}

void disable_uart_rx_interrupt() {
    *AUX_MU_IER &= ~(1);
}

void enable_interrupt() {
    asm volatile("msr DAIFClr, 0xf");
}

void disable_interrupt() {
    asm volatile("msr DAIFSet, 0xf");
}

void irq_handler(unsigned long long x0) {
    if (*CORE0_INT_SRC & CORE0_INT_SRC_GPU && *IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT) { // uart interrupt
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
    uart_printf("spsr_el1: %d, elr_el1: %d, esr_el1: %d", spsr, elr, esr);
}

// output
void uart_tx_interrupt_handler() {
    // buffer empty
    if (tx_bf_r_idx == tx_bf_w_idx) {
        disable_uart_tx_interrupt();
        return;
    }

    *AUX_MU_IO = (unsigned int)tx_buffer[tx_bf_r_idx];
    tx_bf_r_idx++;
    tx_bf_r_idx %= MAX_BUF_SIZE;

    enable_uart_tx_interrupt();
}

void uart_async_putc(char c) {
    // buffer full -> wait for empty space
    while ((tx_bf_w_idx + 1) % MAX_BUF_SIZE == tx_bf_r_idx) {
        enable_uart_tx_interrupt();
    }

    disable_interrupt();
    tx_buffer[tx_bf_w_idx] = c;
    tx_bf_w_idx++;
    tx_bf_w_idx %= MAX_BUF_SIZE;
    enable_interrupt();

    enable_uart_tx_interrupt();
}

// input
void uart_rx_interrupt_handler() {
    // buffer full -> wait for empty space
    if ((rx_bf_w_idx + 1) % MAX_BUF_SIZE == rx_bf_r_idx) {
        disable_uart_rx_interrupt();
        return;
    }
    
    disable_interrupt();
    rx_buffer[rx_bf_w_idx] = (char)(*AUX_MU_IO);
    rx_bf_w_idx++;
    rx_bf_w_idx %= MAX_BUF_SIZE;
    enable_interrupt();
}

char uart_async_getc() {
    // buffer empty -> wait
    while (rx_bf_r_idx == rx_bf_w_idx) {
        enable_uart_rx_interrupt();
    }

    char c = rx_buffer[rx_bf_r_idx];
    rx_bf_r_idx++;
    rx_bf_r_idx %= MAX_BUF_SIZE;

    return c;
}

void set_cpacr_el1(){
    asm volatile(
        "mov x1, (3 << 20)\n\t"
        "msr CPACR_EL1, x1\n\t"
    );
}

