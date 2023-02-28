#include "gpio.h"
#include "aux.h"
#include "helper.h"

void wait_cycles(unsigned int number_of_cycle)
{
    while (number_of_cycle--)
    {
        nop();
    }
}

void uart_init()
{
    // change alternate function of GPIO 14,15
    register unsigned int r = *GPFSEL1;
    // r &= ~((7 << 12) | (7 << 15)); // gpio14, gpio15
    r |= (2 << 12) | (2 << 15); // 010 => alt5
    *GPFSEL1 = r;

    // disable pull-up/down by configure GPPUD and GPPUDCLKn
    *GPPUD = 0;
    wait_cycles(150);
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    wait_cycles(150);
    // Should write to GPPUD to remove the control signal here, but it was 0 already
    *GPPUDCLK0 = 0;

    *AUX_ENABLES |= 1;      // Enable mini UART
    *AUX_MU_CNTL_REG = 0;   // Disable transmitter and receiver
    *AUX_MU_IER_REG = 0;    // Disable interrupt
    *AUX_MU_LCR_REG = 3;    // Set the data size to 8 bit
    *AUX_MU_MCR_REG = 0;    // Disable auto flow control
    *AUX_MU_BAUD_REG = 270; // Set baud rate to 115200
    *AUX_MU_IIR_REG = 6;    // No FIFO
    *AUX_MU_CNTL_REG = 3;   // Enable the transmitter and receiver.
}

void uart_write(unsigned int character)
{
    // wait until field empty
    do
    {
        nop();
    } while (!(*AUX_MU_LSR_REG & 0x20));

    *AUX_MU_IO_REG = character;
}

char uart_read()
{
    // wait until data's ready
    do
    {
        nop();
    } while (!(*AUX_MU_LSR_REG & 0x01));

    char r = (char)(*AUX_MU_IO_REG);

    // convert newline
    return r == '\r' ? '\n' : r;
}

void uart_puts(char *string)
{
    while (*string)
    {
        // convert newline
        if (*string == '\n')
        {
            uart_write('\r');
            uart_write('\n');
        }

        uart_write(*string++);
    }
}

void uart_hex(unsigned int d)
{
    unsigned int n;
    int c;
    for (c = 28; c >= 0; c -= 4)
    {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_write(n);
    }
}
