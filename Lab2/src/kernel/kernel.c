#include "mini_uart.h"
#include "shell.h"
#include "mbox.h"

#include "device_tree.h"
extern void *_dtb_ptr;

void kernel_main(void)
{
	uart_init();

	mbox_main();

	// fdt_traverse(initramfs_callback);

	shell_start();
}
