#include "uart.h"

void delay(int waits) {
    while (waits--);
}

void uart_init() {

    register unsigned int r;

    /*  Configure GPFSELn register to change alternate function.  */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));  // Reset GPIO 14, 15
    r |=   (2 << 12) | (2 << 15);   // Set ALT5
    *GPFSEL1 = r;

    /*  Configure pull up/down register to disable GPIO pull up/down.  */
    *GPPUD = 0;  // Set control signal to disable
    delay(150);  // Wait, provide the required set-up time for the control signal
    *GPPUDCLK0 = (1 << 14) | (1 << 15);  // Clock the control signal into the GPIO pads
    delay(150);
    *GPPUDCLK0 = 0;  // Remove the clock

    /*  Initialization  */
    *AUX_ENABLES     |= 1;    // Enable mini UART
    *AUX_MU_CNTL_REG  = 0;    // Disable TX and RX during configuration
    *AUX_MU_IER_REG   = 0;    // Disable interrupt 
    *AUX_MU_LCR_REG   = 3;    // Set the data size to 8 bit
    *AUX_MU_MCR_REG   = 0;    // Don't need auto flow control
    *AUX_MU_BAUD      = 270;  // Set baud rate to 115200. After booting, the system clock is 250 MHz
    *AUX_MU_IIR_REG   = 6;    // No FIFO
    *AUX_MU_CNTL_REG  = 3;    // Enable Tx and Rx
}

char uart_read(void) {
    while(!(*AUX_MU_LSR_REG & 0x01)) delay(1); // Check data ready field
    char c = (char)(*AUX_MU_IO_REG); // Read
    return (c == '\r') ? '\n' : c; // Convert carrige return to newline
}

void uart_write(unsigned int c) {
    while(!(*AUX_MU_LSR_REG & 0x20)) delay(1); // Check data transmitter field
    *AUX_MU_IO_REG = c;  // Write
}

/* Display a string  */
void uart_puts(char *s) {
    while(*s) {
        // Convert newline to carrige return and newline 
        if(*s=='\n')  uart_write('\r');
        uart_write(*s++);
    }
}
