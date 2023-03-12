#include "mini_uart.h"
#include "utils.h"

void bootloader_main(void)
{
	uart_init();
	uart_send_string("Bootloader Starts...\r\n");

	int img_size = uart_recv_int();
	uart_send_string("Kernel size: ");
	uart_send_int(img_size);
	uart_endl();

	if (img_size <= 0) {
		uart_send_string("Failed\r\n");
		while (1) {}
	}

	uart_send_string("Receiveing kernel...\r\n");
	char *adr = (char*)0x80000;
	for (int i = 0; i < img_size; i++) {
		char c = uart_recv();
		adr[i] = c;
	}

	branch_to_address((void *)0x80000);

	while (1) {}
}