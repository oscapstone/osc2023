#include "gpio.h"
#include "muart.h"

void delay(int waits) {
    while (waits--);
}

void mini_uart_init(void) {
    // configure register to change alternate function
    *GPFSEL1 &= ~((0x7 << 12) | (0x7 << 15));
    *GPFSEL1 |=  ((0x2 << 12) | (0x2 << 15));
    
    // configure pull up/down register to disable GPIO pull up/down
    *GPPUD = 0; delay(150);
    *GPPUDCLK0 = (1 << 14) || (1 << 15); delay(150);

    *AUX_ENABLE      |= 1;      // enable mini UART and its register can be accessed
    *AUX_MU_CNTL_REG  = 0;      // disable transmitter and receiver during configuration
    *AUX_MU_IER_REG   = 0;      // disable interrupt because currently do not need it
    *AUX_MU_LCR_REG   = 3;      // set the data size to 8 bit
    *AUX_MU_MCR_REG   = 0;      // do not need auto flow control
    *AUX_MU_BAUD      = 270;    // set baud rate to 115200
    *AUX_MU_IIR_REG   = 6;      // no FIFO
    *AUX_MU_CNTL_REG  = 3;      // enable the transmitter and recevier
}

char mini_uart_getc(void) {
    while ((*AUX_MU_LSR_REG & 0x01) == 0);
    return (char) *AUX_MU_IO_REG;
}

void mini_uart_gets(char *buffer, int size) {
    char *p = buffer;

    for (int i = 0; i < size - 1; i++) {
        char c = mini_uart_getc();
        if (c == '\r' || c == '\n') {
            mini_uart_puts("\r\n"); break;
        } else if (c < 31 || c > 128) {
            continue;
        }
        mini_uart_putc(c); *p++ = c;
    }

    *p = '\0';
}

void mini_uart_putc(char c) {
    while ((*AUX_MU_LSR_REG & 0x20) == 0);
    *AUX_MU_IO_REG = c;
}

void mini_uart_puts(char *s) {
    while (*s != '\0') {
        mini_uart_putc(*s++);
    }
}
