#include "mini_uart.h"
#include "shell.h"
#include "cpio.h"
#include "devicetree.h"
#include "exception.h"

int main()
{
	uart_init();
	get_devicetree_ptr();
	fdt_traverse(initramfs_callback);

	
	uart_puts("Hi, this is kernel!\r\n");

	shell_start();

	return 0;
}
