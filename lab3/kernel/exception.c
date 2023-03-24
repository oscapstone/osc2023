#include "mini_uart.h"
#include "utils.h"

void print_exception_info(void)
{
        uart_send_string("Exception info ----------\r\n");
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

        uart_send_string("-------------------------\r\n");
        delay(1000);
}