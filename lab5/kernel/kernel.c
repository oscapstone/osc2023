#include "mini_uart.h"
#include "shell.h"
#include "device_tree.h"
#include "exception.h"
#include "timer.h"
#include "mem_frame.h"
#include "mem_allocator.h"
#include "ramdisk.h"
#include "mem_utils.h"
#include "thread.h"

#define MAX_ULONG       0x7fffffffffffffffll

extern char heap_start;
static char *kernel_end = &heap_start;

void kernel_main(void)
{
	set_exception_vector_table();
	uart_init();
	get_device_tree_adr();

	enable_core_timer();
	reset_core_timer_absolute(MAX_ULONG);
	enable_interrupts_in_el1();

	// TODO: add it back when not using QEMU
	init_ramdisk();

	init_frames();
	init_allocator();
	memory_reserve((void*)0x0, (void*)kernel_end);
	process_mem_reserve();

	init_thread();

	uart_send_string("Kernel Starts...\r\n");
	shell_loop();
}