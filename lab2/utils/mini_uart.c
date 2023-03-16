#include "utils.h"
#include "peripherals/gpio.h"
#include "peripherals/mini_uart.h"

void uart_init(){
    /* the following lines of code that are used to configure GPIO pins 14 and 15 to work with the Mini UART device */
    unsigned int selector;

    selector = get32(GPFSEL1);
    selector &= ~(7<<12);       // clean gpio 14
    selector |= 2<<12;          // set alt5 at gpio14
    selector &= ~(7<<15);       // clean gpio 15
    selector |= 2<<15;          // set alt5 at gpio 15
    put32(GPFSEL1, selector);

    /*we can remove both the pull-up and pull-down states from a pin, which is what we are doing for pins 14 and 15 in the following code*/
    put32(GPPUD, 0);            // disable pull-up/down
    delay(150);                 // wait 150 cycles
    put32(GPPUDCLK0, (1<<14)|(1<<15));  // only modify GPIO 14 and 15
    delay(150);
    put32(GPPUDCLK0, 0);        // remove the clock

    /*Now our Mini UART is connected to the GPIO pins, and the pins are configured. The rest of the uart_init function is dedicated to Mini UART initialization.*/
    put32(AUX_ENABLES, 1);      // enable mini UART
    put32(AUX_MU_CNTL_REG, 0);  // Disable transmitter and receiver during configuration
    put32(AUX_MU_IER_REG, 0);   // Disable interrupt
    put32(AUX_MU_LCR_REG, 3);   // Set the data size to 8 bit
    put32(AUX_MU_MCR_REG, 0);   // Don’t need auto flow control
    put32(AUX_MU_BAUD_REG, 270);    // Set baud rate to 115200

    put32(AUX_MU_IIR_REG, 6);   // No FIFO
    put32(AUX_MU_CNTL_REG, 3);  // Enable the transmitter and receiver
}

/*
Here, we use the two functions put32 and get32. Those functions are very simple; 
they allow us to read and write some data to and from a 32-bit register. 
You can take a look at how they are implemented in 'utils.S'.
*/


/*Use while loop to verify whether the device is ready to transmit or receive data. We are using the AUX_MU_LSR_REG register to do this. */
void uart_putc(char c){
    while(!(get32(AUX_MU_LSR_REG)&0x20));
    put32(AUX_MU_IO_REG, c);
}

char uart_getc(void){
    while(!(get32(AUX_MU_LSR_REG)&0x01));
    return get32(AUX_MU_IO_REG);
}

void uart_puts(char* str){
    while(*str){
        uart_putc(*str);
        str += 1;
    }
}

void uart_gets(char *buffer, int size) {
    char *p = buffer;

    for (int i = 0; i < size - 1; i++) {
        char c = uart_getc();
        if (c == '\r' || c == '\n') {
            uart_puts("\r\n"); break;
        } else if (c < 31 || c > 128) {
            continue;
        }
        uart_putc(c); *p++ = c;
    }

    *p = '\0';
}

/*REF: https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md */