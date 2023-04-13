#include "mini_uart.h"
#include "shell.h"
#include "mbox.h"
#include "page_alloc.h"
#include "dynamic_alloc.h"
#include "device_tree.h"
extern void *_dtb_ptr;

void kernel_main(void)
{
	uart_init();

	mbox_main();

	fdt_traverse(initramfs_callback, _dtb_ptr);

	init_page_frame();
	init_pool();
	shell_start();
}
