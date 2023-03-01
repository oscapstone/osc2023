#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

void uart_send(char c)
{
	/*
	 * Bit five, if set to 1, meaning that we can write to the UART.
	 */
	while (!(get32(AUX_MU_LSR_REG) & 0x20)) {}
	put32(AUX_MU_IO_REG, c);
}

char uart_recv(void)
{
	/*
	 * Bit zero, if set to 1, indicates that the data is ready.
	 */
	while (!(get32(AUX_MU_LSR_REG) & 0x01)) {}
	return (get32(AUX_MU_IO_REG) & 0xFF);
}

/*
 * Iterates over all characters in a string and sends them one by one.
 */
void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i++) {
		uart_send((char)str[i]);
	}
}

void uart_send_hex(unsigned int n)
{
    char out[9];
    out[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        unsigned int o = n % 16;
        n >>= 4;
        switch (o) {
        case 10:
            out[i] = 'A';
            break;
        case 11:
            out[i] = 'B';
            break;
        case 12:
            out[i] = 'C';
            break;
        case 13:
            out[i] = 'D';
            break;
        case 14:
            out[i] = 'E';
            break;
        case 15:
            out[i] = 'F';
            break;
        default:
            out[i] = '0' + o;
            break;
        }
    }
    uart_send_string("0x");
    uart_send_string(out);
}

void uart_init(void)
{
	unsigned int selector;

	/*
	 * Bit 12-14 is for GPIO 14, it 15-17 is for GPIO 15.
	 * Set them to 2 for alt5.
	 */
	selector = get32(GPFSEL1);

	selector &= ~(7 << 12);
	selector |= (2 << 12);

	selector &= ~(7 << 15);
	selector |= (2 << 15);

	put32(GPFSEL1, selector);

	/*
	 * Sets GPIO pull-up/down
	 *
	 * 00 = Off – disable pull-up/down
	 * 01 = Enable Pull Down control
	 * 10 = Enable Pull Up control
	 * 11 = Reserved 
	 */
	put32(GPPUD, 0);
	delay(150);
	put32(GPPUDCLK0, (1 << 14) | (1 << 15));  // Asserts Clock
	delay(150);
	put32(GPPUDCLK0, 0);  // Removes Clock

	/*
	 * Initializes mini uart (this also enables access to its registers)
	 */
	put32(AUX_ENABLES, 1);
	/*
	 * Disables auto flow control, disables receiver and transmitter
	 * before the configuration is finished
	  */
	put32(AUX_MU_CNTL_REG, 0);
	/*
	 * Disables receive and transmit interrupts
	 */
	put32(AUX_MU_IER_REG, 0);
	/*
	 * Mini UART can support either 7- or 8-bit operations.
	 * Enables 8 bit mode here.
	 */
	put32(AUX_MU_LCR_REG, 3);
	/*
	 * The RTS line is used in the flow control and we don't need it.
	 * Sets it to be high all the time.
	 */
	put32(AUX_MU_MCR_REG, 0);
	/*
	 * Sets baud rate to 115200, a maximum of 115200 bits per second.
	 * baudrate = system_clock_freq / (8 * ( baudrate_reg + 1 ))
	 */
	put32(AUX_MU_BAUD_REG, 270);

	// TODO: what is this???
	// Disable interrupts
	// put32(AUX_MU_IIR_REG, 6);

	/*
	 * Enables transmitter and receiver
	 */
	put32(AUX_MU_CNTL_REG, 3);
}