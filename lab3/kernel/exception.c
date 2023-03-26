#include "mini_uart.h"
#include "uart_async.h"
#include "utils.h"
#include "timer.h"
#include "peripherals/exception.h"
#include "peripherals/mini_uart.h"

void print_exception_info(void)
{
        uart_send_string("Exception info -------------------\r\n");
        unsigned long tmp;

        uart_send_string("spsr_el1:\t");
        asm volatile("mrs %0, spsr_el1": "=r" (tmp));
        uart_send_hex_64(tmp);
        uart_endl();

        uart_send_string("elr_el1:\t");
        asm volatile("mrs %0, elr_el1": "=r" (tmp));
        uart_send_hex_64(tmp);
        uart_endl();

        uart_send_string("esr_el1:\t");
        asm volatile("mrs %0, esr_el1": "=r" (tmp));
        uart_send_hex_64(tmp);
        uart_endl();

        uart_send_string("----------------------------------\r\n");
}

void el0_timer_interrupt()
{
        uart_send_string("Timer interrupt info -------------\r\n");
        print_current_time();
        uart_send_string("----------------------------------\r\n");
        reset_core_timer_in_second(2);
}

void gpu_interrupt(void)
{
        // TODO: comment Aux int
        if (get32(IRQ_PENDING_1) & (1 << 29)) {
                uart_async_interrupt_handler();
        }
}

void irq_64_el0(void)
{
        unsigned int interrupt_source = get32(CORE0_IRQ_SOURCE);
        if (interrupt_source & (1 << 8)) {
                gpu_interrupt();
        } else {
                el0_timer_interrupt();
        }
}

void enable_2nd_level_interrupt_ctrl(void)
{
        unsigned int tmp = get32(ENABLE_IRQs1);
        tmp |= (1 << 29);
        put32(ENABLE_IRQs1, tmp);
}

// TODO
// void disable_uart_interrupt() { put32(DISABLE_IRQS_1, (1<<29)); }