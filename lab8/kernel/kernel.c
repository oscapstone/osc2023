#include "mini_uart.h"
#include "shell.h"
#include "devtree.h"
#include "cpio.h"
#include "mm.h"
#include "timer.h"
#include "exception.h"
#include "fork.h"
#include "vfs.h"
#include "sdhost.h"

void kernel_main(void)
{
	uart_init();
	devtree_getaddr();
	fdt_traverse(initramfs_callback);
	init_mm_reserve();
	timer_init();
	rootfs_init();
	initramfs_init();
	sd_init();
	fat32_init();
	enable_interrupt();
	uart_send_string("OSDI 2022 Spring\n");

	copy_process(PF_KTHREAD, (unsigned long)&shell_loop, 0, 0);

	while (1) {
		kill_zombies();
		schedule();
	}
}