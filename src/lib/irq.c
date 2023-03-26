#include <irq.h>
#include <mini_uart.h>
#include <utils.h>
#include <timer.h>
#include <BCM.h>

void default_exception_handler(uint32 n){
    uart_printf("[exception] %d\r\n", n);
}

void irq_handler(){
    timer_irq_handler();
    uart_irq_handler();
}

void enable_irqs1(){
    put32(ENABLE_IRQS1, 1 << 29);             // Enable UART1 IRQ
}