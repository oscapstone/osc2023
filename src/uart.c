#include "aux.h"
#include "gpio.h"
#include "my_string.h"

void uart_init() {
    /* Initialize UART */
    *AUX_ENABLES |= 1;              // Enable mini uart (this also enables access to all mini-uart registers)
    *AUX_MU_CNTL = 0;               // Disable auto flow control because it requires us to use additional GPIO pins not supported here,
                                    // and disable receiver and transmitter (for now)
    *AUX_MU_IER = 0;                // Disable receive and transmit interrupts, we gonna use this function afterward.
    *AUX_MU_LCR = 3;                // Enable 8 bit mode (7 or 8)
    *AUX_MU_MCR = 0;                // Set RTS line used for flow control to be always high, we don't need it.
    *AUX_MU_BAUD = 270;             // Set baud rate to 115200 (maximum of 115200 bits per second)
    *AUX_MU_IIR = 6;                // No FIFO

    /* Map UART to GPIO Pins */

    // 1. Change GPIO 14, 15 to alternate function
    register unsigned int r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15));  // clean gpio14         REGISTER && 000111111111111
                                    // clean gpio15         REGISTER && 000111111111111111
    r |= (2 << 12) | (2 << 15);     // set alt5 for gpio14  REGISTER || 010000000000000
                                    // set alt5 for gpio15  REGISTER || 010000000000000000
    *GPFSEL1 = r;

    // 2. Disable GPIO pull up/down (Because these GPIO pins use alternate functions, not basic input-output)
    *GPPUD = 0;                             // Write 0 means wanna remove pull-up/down
                                            // Wait 150 cycles
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);     // Remove for which pins
    // Wait 150 cycles
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = 0;                         // Remove clock

    // 3. Enable TX, RX
    *AUX_MU_CNTL = 3;
}

char uart_read() {
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));        // AUX_MU_LSR shows the data status
    char r = (char)(*AUX_MU_IO);            // Write data to and read data from the UART FIFOs
    return r == '\r' ? '\n' : r;
}

void uart_write(unsigned int c) {
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));
    *AUX_MU_IO = c;
}

void uart_printf(char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    extern volatile unsigned char _end;  // defined in linker
    char* s = (char*)&_end;              // put temporary string after code
    vsprintf(s, fmt, args);

    while (*s) {
        if (*s == '\n') uart_write('\r');
        uart_write(*s++);
    }
}

void uart_flush() {
    while (*AUX_MU_LSR & 0x01) {
        *AUX_MU_IO;
    }
}

void uart_2hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c = 28; c >= 0; c -= 4) {
        n = (d >> c) & 0xF;
        n += n > 9 ? 0x37 : 0x30;
        uart_write(n);
    }
}