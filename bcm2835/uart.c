#include "bcm2835/gpio.h"
#include "bcm2835/uart.h"

static inline void wait_cycles(unsigned int i) {
    while (i--) {
        asm volatile("nop");
    }
}

void uart_init() {
    *AUX_ENABLES     = 1;   // Enable mini UART only
    *AUX_MU_CNTL_REG = 0;   /* Disable transmitter and receiver
                               during configuration */

    *AUX_MU_IER_REG  = 0;   // Disable interrupt
    *AUX_MU_LCR_REG  = 3;   // Set the data size to 8 bit
    *AUX_MU_MCR_REG  = 0;   // Disable auto flow control
    *AUX_MU_BAUD_REG = 270; // Set baud rate to 115200

    // Clear FSEL14 and FSEL15 (GPIO 14, 15)
    *GPFSEL1 &= ~((7 << 12) | (7 << 15));
    // Set ALT5 on FSEL14 and FSEL15
    *GPFSEL1 |= (2 << 12) | (2 << 15);
    
    // Disable GPIO pull up/down
    *GPPUD = 0;
    // Wait 150 cycles
    wait_cycles(150);
    // GPIO 14, 15
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    // Wait 150 cycles
    wait_cycles(150);
    // Remove the control signal
    // *GPPUD = 0;
    // Remove the clock
    *GPPUDCLK0 = 0;

    *AUX_MU_IIR_REG  = 6;   // Disable FIFO
    *AUX_MU_CNTL_REG = 3;   // Enable transmitter (Tx) and receiver(Rx)
}

char uart_recv() {
    while (!(*AUX_MU_LSR_REG & (1 << 0))) {/* nop */}
    char c = (char)(*AUX_MU_IO_REG);
    return c == '\r' ? '\n' : c;
}

void uart_send(char c) {
    if (c == '\n') {
        uart_send('\r');
    }
    while (!(*AUX_MU_LSR_REG & (1 << 5))) {/* nop */}
    *AUX_MU_IO_REG = c;
}

void uart_puts(char * s) {
    while (*s) {
        uart_send(*s++);
    }
}
