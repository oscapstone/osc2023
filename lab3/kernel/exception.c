#include "mini_uart.h"
#include "uart_async.h"
#include "utils.h"
#include "timer.h"
#include "peripherals/exception.h"
#include "peripherals/mini_uart.h"

void enable_interrupts_in_el1(void)
{
        asm volatile("msr DAIFClr, 0xf");
}

void disable_interrupts_in_el1(void)
{
        asm volatile("msr DAIFSet, 0xf");
}

void print_exception_info(void)
{
        disable_interrupts_in_el1();

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

        enable_interrupts_in_el1();
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
        /*
         * AUX INT (keyboard, mouse, or other user interactions.)
         */
        if (get32(IRQ_PENDING_1) & (1 << 29)) {
                uart_async_interrupt_handler();
        }
}

void irq_64_el0(void)
{
        disable_interrupts_in_el1();

        unsigned int interrupt_source = get32(CORE0_IRQ_SOURCE);
        if (interrupt_source & (1 << 8)) {
                gpu_interrupt();
        } else if (interrupt_source & (1 << 1)) {
                /*
                 * CNTPNSIRQ interrupt
                 */
                el0_timer_interrupt();
        }

        enable_interrupts_in_el1();
}

void irq_64_el1(void)
{
        disable_interrupts_in_el1();
        unsigned int interrupt_source = get32(CORE0_IRQ_SOURCE);
        if (interrupt_source & (1 << 1)) {
                /*
                 * CNTPNSIRQ interrupt
                 */
                el1_timer_handler();
        } else if (interrupt_source & (1 << 8)) {
                gpu_interrupt();
        }
        enable_interrupts_in_el1();
}