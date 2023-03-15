#include "mini_uart.h"
#include "shell.h"
#include "mbox.h"

#include "device_tree.h"
extern void *_dtb_ptr;

#include "stdlib.h"

void kernel_main(void)
{
	uart_init();

	mbox_main();
	printf("_dtb_ptr = ");
	printf("%p", _dtb_ptr);
	printf("\n");
	// fdt_traverse(_dtb_ptr);
	printf("end traverse\n");
	shell_start();
}
