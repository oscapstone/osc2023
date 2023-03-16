#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

void uart_send(char c)
{
	/*
	 * Bit five, if set to 1, meaning that we can write to the UART.
	 */
	while (!(get32(AUX_MU_LSR_REG) & 0x20)) { asm volatile("nop"); }
	put32(AUX_MU_IO_REG, c);
}

/*
 * Receive 1 byte from uart
 */
char uart_recv(void)
{
	/*
	 * Bit zero, if set to 1, indicates that the data is ready.
	 */
	while (!(get32(AUX_MU_LSR_REG) & 0x01)) { asm volatile("nop"); }
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

int uart_recv_int(void)
{
	int num = 0;
	for (int i = 0; i < 4; i++) {
		char c = uart_recv();
		num <<= 8;
		num += (int)c;
	}
	return num;
}

void uart_send_int(int num)
{
	char out[12];
	int i = 10;

	int neg = 0;
	if (num < 0) {
		neg = 1;
		num = -num;
	}

	out[11] = '\0';
	do {
		out[i] = '0' + (num % 10);
		num /= 10;
		i--;
	} while (num > 0);

	if (neg) {
		out[i] = '-';
	} else {
		i++;
	}

	uart_send_string(&out[i]);
}

void uart_endl(void)
{
	uart_send('\r');
	uart_send('\n');
}

void uart_readline(char* buffer, int len)
{
	unsigned int idx = 0;
        char c = '\0';
        
        while (1) {
                c = uart_recv();
                if (c == '\r' || c == '\n') {
                        uart_endl();
                        
                        if (idx < len) buffer[idx] = '\0';
                        else buffer[len - 1] = '\0';
                        
                        break;
                } else {
                        uart_send(c);
                        buffer[idx++] = c;
                } 
        }
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

	/*
	 * Interrupt identify no FIFO.
	 */
	put32(AUX_MU_IIR_REG, 6);

	/*
	 * Enables transmitter and receiver
	 */
	put32(AUX_MU_CNTL_REG, 3);
}