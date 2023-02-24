#include "mini_uart.h"
#include "gpio.h"
#include "utils.h"
#include "mailbox.h"
#include "uart.h"


void uart_init(void) 
{
    
    /* set alternative function */
    //https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md#mini-uart-initialization
    //register keyword ask compiler to store variable in CPU register as possible.
    register unsigned int selector = *GPFSEL1;
    selector &= ~(7<<12);                   // clean gpio14
    selector |= 2<<12;                      // set alt5 for gpio14
    selector &= ~(7<<15);                   // clean gpio15
    selector |= 2<<15;                      // set alt5 for gpio 15
    *GPFSEL1 = selector;

    /* we need neither the pull-up nor the pull-down state, because both the 14 and 15 pins are going to be connected all the time. */
    *GPPUD = 0;
    delay(150);
    *GPPUDCLK0 = ((1<<14) | (1<<15));
    delay(150);
    *GPPUDCLK0 = 0;

    /* Initializing the Mini UART */
    *AUX_ENABLES = 1;               //Enable mini uart (this also enables access to its registers)
    *AUX_MU_CNTL = 0;               //Disable auto flow control and disable receiver and transmitter (for now)
    *AUX_MU_IER = 0;                //Disable receive and transmit interrupts
    *AUX_MU_LCR = 3;                //Enable 8 bit mode
    *AUX_MU_MCR = 0;                //Set RTS line to be always high
    *AUX_MU_BAUD = 270;             //Set baud rate to 115200

    *AUX_MU_CNTL = 3;               //Finally, enable transmitter and receiver
}

char uart_read(void)
{
    while (!(*AUX_MU_LSR & 0x01));
    char r = *AUX_MU_IO & 0xff;
    return r == '\r' ? '\n' : r;
}

unsigned int uart_readline(char *buffer, unsigned int buffer_size)
{
    if (buffer_size == 0) return 0;
    char *ptr = buffer, *buffer_tail = buffer + buffer_size - 1;
    do {
        *ptr = uart_read();
    } while (*ptr != '\n' && (ptr++ < buffer_tail));
    *ptr = '\0';
    return (ptr - buffer);
}

void uart_write(char c)
{
    while (!(*AUX_MU_LSR & 0x20));
    *AUX_MU_IO = c;
}

void uart_flush() {
    while (*AUX_MU_LSR & 0x01) {
        *AUX_MU_IO;
    }
}

void uart_write_string(char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        uart_write((char)str[i]);
    }
}

void uart_write_no(unsigned int n) 
{
    if (n < 10) {
        uart_write('0' + n);
        return;
    } else {
        uart_write_no(n / 10);
        uart_write('0' + n % 10);
    }
}

void uart_write_no_hex(unsigned int n)
{
    const char *hex_str = "0123456789abcdef";
    if (n < 16) {
        uart_write(hex_str[n]);
    } else {
        uart_write_no_hex(n >> 4);
        uart_write(hex_str[n & 0xf]);
    }
}