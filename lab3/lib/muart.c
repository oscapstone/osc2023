#include "gpio.h"
#include "muart.h"
#include "utils.h"
#include "exception.h"

char rbuffer[BUFSIZE], wbuffer[BUFSIZE];
unsigned int rstart = 0, rend = 0, wstart = 0, wend = 0;

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

void mini_uart_gets(char *buffer, unsigned int size) {
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

void mini_uart_putc(const char c) {
    while ((*AUX_MU_LSR_REG & 0x20) == 0);
    *AUX_MU_IO_REG = c;
}

void mini_uart_puts(const char *s) {
    while (*s != '\0') {
        mini_uart_putc(*s++);
    }
}

void enable_read_interrupt(void) {
    *AUX_MU_IER_REG |= 0x01;
}

void disable_read_interrupt(void) {
    *AUX_MU_IER_REG &= ~0x01;
}

void enable_write_interrupt(void) {
    *AUX_MU_IER_REG |= 0x02;
}

void disable_write_interrupt(void) {
    *AUX_MU_IER_REG &= ~0x02;
}

void enable_mini_uart_interrupt(void) {
    enable_read_interrupt();
    *ENABLE_IRQS_1 |= (1 << 29);
}

void disable_mini_uart_interrupt(void) {
    disable_write_interrupt();
    *DISABLE_IRQS_1 |= (1 << 29);
}

void async_mini_uart_handler(void) {
    disable_mini_uart_interrupt();

    if (*AUX_MU_IIR_REG & 0x04) {
        char c = *AUX_MU_IO_REG;
        rbuffer[rend++] = c;
        rend = (rend == BUFSIZE)? 0: rend;
    } else if (*AUX_MU_IIR_REG & 0x02) {
        while (*AUX_MU_LSR_REG & 0x20) {
            if (wstart == wend) {             
                enable_read_interrupt(); break;
            }

            char c = wbuffer[wstart++];
            *AUX_MU_IO_REG = c;
            wstart = (wstart == BUFSIZE)? 0: wstart;
        }
    }

    enable_mini_uart_interrupt();
}

void async_mini_uart_puts(const char *s) {
    while (*s != '\0') {
        wbuffer[wend++] = *s++;
        wend = (wend == BUFSIZE)? 0: wend;
    }

    enable_write_interrupt();
}

unsigned int async_mini_uart_gets(char *buffer, unsigned int size) {
    int idx;

    for (idx = 0; idx < size - 1; idx++) {
        while (rstart == rend) {
            asm volatile("nop\r\n");
        }

        if (rbuffer[rstart] == '\r' || rbuffer[rstart] == '\n') {
            rstart += (rstart + 1 >= BUFSIZE)? 1 - BUFSIZE: 1;
            break;
        }

        buffer[idx] = rbuffer[rstart++];
        rstart = (rstart == BUFSIZE)? 0: rstart;
    }

    buffer[idx] = '\0';

    return idx;
}