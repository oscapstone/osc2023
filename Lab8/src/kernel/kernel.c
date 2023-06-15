#include "mini_uart.h"
#include "shell.h"
#include "mbox.h"
#include "reserve_mem.h"
#include "device_tree.h"
#include "thread.h"
#include "virtual_mem.h"
#include "vfs.h"
#include "tmpfs.h"
#include "stdlib.h"
#include "sdhost.h"

extern void *_dtb_ptr;

void kernel_main(void)
{
	uart_init();

	mbox_main();

	fdt_traverse(initramfs_callback, _dtb_ptr);

	thread_init();

	memory_init();

	// virtual_mem_init();

	vfs_mount("/", "tmpfs");
	vfs_mount("/initramfs", "initramfs");
	vfs_mount("/dev", "devfs");
	strcpy(cwdpath, rootfs->root->internal->name);

	sd_init();
	vfs_mount("/boot", "fat32fs");

	shell_start();
}
