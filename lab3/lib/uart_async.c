#include "utils.h"
#include "mini_uart.h"
#include "string_utils.h"
#include "peripherals/mini_uart.h"

#define BUFFER_SIZE 256

char read_buffer[BUFFER_SIZE];  // circular queue TODO: comment
int read_st = 0, read_ed = 0;
char write_buffer[BUFFER_SIZE];
int write_st = 0, write_ed = 0;

void enable_transmit_interrupt(void)
{
	unsigned int tmp = get32(AUX_MU_IER_REG);
	tmp |= 0x2;
	put32(AUX_MU_IER_REG, tmp);
}

void disable_transmit_interrupt(void)
{
	unsigned int tmp = get32(AUX_MU_IER_REG);
	tmp &= ~(0x2);
	put32(AUX_MU_IER_REG, tmp);
}

/*
 * Return 0 on exceeding buffer length.
 */
int uart_async_send_string(char* str)
{
	if (strlen(str) >= BUFFER_SIZE-1) return 0;
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\n') {
			write_buffer[write_ed++] = '\r';
			write_ed %= BUFFER_SIZE;
			write_buffer[write_ed++] = '\n';
			write_ed %= BUFFER_SIZE;
		} else {
			write_buffer[write_ed++] = str[i];
			write_ed %= BUFFER_SIZE;
		}
	}
	enable_transmit_interrupt();
	return 1;
}

void uart_send_buffer_content(void)
{
	if (write_st != write_ed) {
		uart_send(write_buffer[write_st++]);
		write_st %= BUFFER_SIZE;
	}
	if (write_st == write_ed) {
		disable_transmit_interrupt();
		return;
	}
}

void uart_receive_to_buffer(void)
{
	char c = uart_recv();
	read_buffer[read_ed++] = c;
	read_ed %= BUFFER_SIZE;
	if (c == '\r') uart_send('\n');
	else uart_send(c);
}

int uart_async_readline(char* target, int len)
{
	int i;
	len -= 1; // Reserves space for '\0'
	for (i = 0; i < len; i++) {
		while(read_st == read_ed) { asm volatile("nop"); }
		char c = read_buffer[read_st++];
		read_st %= BUFFER_SIZE;

                if (c == '\r' || c == '\n') {
			target[i] = '\0';
                        break;
                } else {
                        target[i] = c;
                }
	}
	target[i] = '\0';
	return i;
}

void demo_uart_async(void)
{
	enable_transmit_interrupt();
	delay(1000);

	char buffer[BUFFER_SIZE];
	uart_async_send_string("Type something > ");
	uart_async_readline(buffer, BUFFER_SIZE);
	uart_async_send_string("[Received] ");
	uart_async_send_string(buffer);
	uart_async_send_string("\r\n");

	delay(1000);
	while(1) { asm volatile("nop"); }
}

void uart_async_interrupt_handler(void)
{
        unsigned int interrupt_source = get32(AUX_MU_IIR_REG);
        if (interrupt_source & (0x2)) {
                uart_send_buffer_content();
        } else if (interrupt_source & (0x4)) {
                uart_receive_to_buffer();
        }
}