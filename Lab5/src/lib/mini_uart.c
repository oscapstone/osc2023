#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "peripherals/irq.h"
#include "stdlib.h"

// get address from linker
extern volatile unsigned char _end;

char read_buffer[100];
char write_buffer[100];
int len_WB = 0;
int len_RB = 0;
int buffer_count = 0;

void uart_send(char c)
{
	while (1)
	{
		if (get32(AUX_MU_LSR_REG) & 0x20)
			break;
	}

	if (c != '\x7f')
		put32(AUX_MU_IO_REG, c);

	if (c == '\n')
		uart_send('\r');
}

char uart_recv(void)
{
	char r;
	while (1)
	{
		if (get32(AUX_MU_LSR_REG) & 0x01)
			break;
	}
	r = get32(AUX_MU_IO_REG);
	return r == '\r' ? '\n' : r;
}

void uart_send_string(char *str)
{
	while (*str)
		uart_send(*str++);
}

void uart_send_space(int size)
{
	while (size--)
		uart_send(' ');
}

void uart_send_string_of_size(char *str, int size)
{
	while (size--)
		uart_send(*str++);
}

/**
 * Display a binary value in hexadecimal
 */
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
		uart_send(n);
	}
}

void uart_init(void)
{
	unsigned int selector;

	selector = get32(GPFSEL1);
	selector &= ~(7 << 12); // clean gpio14
	selector |= 2 << 12;	// set alt5 for gpio14
	selector &= ~(7 << 15); // clean gpio15
	selector |= 2 << 15;	// set alt5 for gpio15
	put32(GPFSEL1, selector);

	put32(GPPUD, 0);
	delay(150); // spec
	put32(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
	put32(GPPUDCLK0, 0);

	put32(AUX_ENABLES, 1);		 // Enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG, 0);	 // Disable auto flow control and disable receiver and transmitter (for now)
	put32(AUX_MU_IER_REG, 0);	 // Disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG, 3);	 // Enable 8 bit mode
	put32(AUX_MU_MCR_REG, 0);	 // Set RTS line to be always high
	put32(AUX_MU_BAUD_REG, 270); // Set baud rate to 115200

	put32(AUX_MU_CNTL_REG, 3); // Finally, enable transmitter and receiver
}

void _putchar(char character)
{
	uart_send(character);
}

/**
 * Display a string
 */
// void printf(char *fmt, ...)
// {
// 	__builtin_va_list args;
// 	__builtin_va_start(args, fmt);
// 	// we don't have memory allocation yet, so we
// 	// simply place our string after our code
// 	char *s = (char *)&_end;
// 	// use sprintf to format our string
// 	vsprintf(s, fmt, args);
// 	// print out as usual
// 	while (*s)
// 	{
// 		/* convert newline to carrige return + newline */
// 		if (*s == '\n')
// 			uart_send('\r');
// 		uart_send(*s++);
// 	}
// }

void asyn_read()
{
	memset(read_buffer, '\0', 100);

	put32(AUX_MU_IER_REG, 1); // Enable receive and transmit interrupts
	put32(ENABLE_IRQS_1, 1 << 29);

	// enable interrupt in el1
	asm("msr DAIFClr, 0xf;");

	while (read_buffer[strlen(read_buffer) - 1] != '\r')
		;
}

void asyn_write(char *str)
{
	strcpy(write_buffer, str);
	len_WB = strlen(write_buffer);

	put32(AUX_MU_IER_REG, 2); // Enable receive and transmit interrupts
	put32(ENABLE_IRQS_1, 1 << 29);

	// enable interrupt in el1
	asm("msr DAIFClr, 0xf;");

	while (strlen(write_buffer) != 0)
		;
}

void uart_rx_handler()
{
	read_buffer[len_RB++] = get32(AUX_MU_IO_REG);

	if (read_buffer[len_RB - 1] == '\r')
	{
		put32(AUX_MU_IER_REG, 0); // Disable receive and transmit interrupts
		read_buffer[len_RB] = '\0';
		len_RB = 0;
	}
}

void uart_tx_handler()
{
	if (buffer_count < len_WB)
		put32(AUX_MU_IO_REG, write_buffer[buffer_count++]);
	else
	{
		put32(AUX_MU_IER_REG, 0); // Disable receive and transmit interrupts
		buffer_count = 0;
		memset(write_buffer, '\0', 100);
	}
}

void enable_uart_irq()
{
	put32(ENABLE_IRQS_1, 1 << 29);
}

void disable_uart_irq()
{
	put32(DISABLE_IRQS_1, 1 << 29);
}