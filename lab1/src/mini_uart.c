#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

void uart_send(char c) {
	while(1) {
		// Bit five, if set to 1, tells us that the transmitter is empty,
		// meaning that we can write to the UART.
		if(get32(AUX_MU_LSR_REG) & 0x20) 
			break;
	}
	put32(AUX_MU_IO_REG, c);
}

char uart_recv(void) {
	while(1) {
		// Bit zero, if set to 1, indicates that the data is ready.
		if(get32(AUX_MU_LSR_REG) & 0x01) 
			break;
	}
	return (get32(AUX_MU_IO_REG) & 0xFF);
}

/* Iterates over all characters in a string and sends them one by one. */
void uart_send_string(char* str) {
	for(int i = 0; str[i] != '\0'; i++) {
		uart_send((char)str[i]);
	}
}

void uart_init(void) {
	unsigned int selector;

	selector = get32(GPFSEL1);
	// Bit 12-14 is for GPIO 14
	selector &= ~(7<<12);
	// Sets alt5
	selector |= 2<<12;
	// Bit 15-17 is for GPIO 15
	selector &= ~(7<<15);
	selector |= 2<<15;
	put32(GPFSEL1, selector);

	// 00 = Off – disable pull-up/down
	// 01 = Enable Pull Down control
	// 10 = Enable Pull Up control
	// 11 = Reserved 
	put32(GPPUD, 0);
	delay(150);
	// GPIO Pull-up/down Clock Registers
	// Assert Clock
	put32(GPPUDCLK0, (1<<14) | (1<<15));
	delay(150);
	// Remove Clock
	put32(GPPUDCLK0, 0);

	// Enable mini uart (this also enables access to its registers)
	put32(AUX_ENABLES, 1);
	// Disable auto flow control and disable receiver and transmitter
	// before the configuration is finished
	put32(AUX_MU_CNTL_REG, 0);
	// Disable receive and transmit interrupts
	put32(AUX_MU_IER_REG, 0);
	// Mini UART can support either 7- or 8-bit operations.
	// Enable 8 bit mode here.
	put32(AUX_MU_LCR_REG, 3);
	// The RTS line is used in the flow control and we don't need it.
	// Set it to be high all the time.
	put32(AUX_MU_MCR_REG, 0);
	// Set baud rate to 115200, which means a maximum of 115200 bits per second.
	// baudrate = system_clock_freq / (8 * ( baudrate_reg + 1 )) 
	put32(AUX_MU_BAUD_REG, 270);

	// TODO: what is this???
	// Disable interrupts
	put32(AUX_MU_IIR_REG, 6);

	// Enable transmitter and receiver
	put32(AUX_MU_CNTL_REG, 3);
}