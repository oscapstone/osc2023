#include <irq.h>
#include <mini_uart.h>
#include <utils.h>
#include <timer.h>

void default_exception_handler(uint32 n){
    uart_printf("[exception] %d\r\n", n);
}

void enable_irq1(){
    put32(ENABLE_IRQS1, 1 << 29);
}

void irq_handler(){
    timer_irq_handler();
    // uart_irq_handler();
}