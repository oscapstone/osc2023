#include "peripherals/mini_uart.h"


char beg[] = "In bootloader!!\r\n";
void main(void) {
    uart_init();

    char *kernel_addr = (char *)0x80000;

	// while (1) {
	// 	uart_send(uart_recv());
	// }
    int kernel_sz = 0;
    for(char *ch = beg; *ch != '\0'; ch ++) {
        uart_send(*ch);
    }
    while(1) {
        char c = uart_recv();
        if(c == 'S') {
            kernel_sz = uart_recv();
            kernel_sz = (kernel_sz << 8) + uart_recv();
            kernel_sz = (kernel_sz << 8) + uart_recv();
            kernel_sz = (kernel_sz << 8) + uart_recv();
            break;
        } else {
            uart_send(c);
        }
    }
    // uart_send_u32(kernel_sz);
    while(kernel_sz --) {
        *(kernel_addr ++) = uart_recv();
    }
    uart_send('F');
    uart_send('I');
    uart_send('N');
    uart_send('I');
    uart_send('S');
    uart_send('H');
    uart_send('\r');
    uart_send('\n');

    // while(1) {
    //     uart_send(uart_recv());
    // }

    asm volatile(
        "mov x0, x10\n"
        "mov x1, x11\n"
        "mov x2, x12\n"
        "mov x3, x13\n"

        "b _second_kernel\n"
    );
}
