#include "mini_uart.h"
#include "shell.h"
#include "mbox.h"
#include "reserve_mem.h"
#include "device_tree.h"
#include "thread.h"
#include "virtual_mem.h"

#include "stdlib.h"

extern void *_dtb_ptr;

void kernel_main(void)
{
	uart_init();

	mbox_main();

	fdt_traverse(initramfs_callback, _dtb_ptr);

	memory_init();

	thread_init();

	shell_start();
}
