#include "mini_uart.h"
#include "shell.h"
#include "device_tree.h"
#include "exception.h"
#include "timer.h"
#include "mem_frame.h"
#include "mem_allocator.h"

#define MAX_ULONG       0x7fffffffffffffffll

void kernel_main(void)
{
	set_exception_vector_table();
	uart_init();
	get_device_tree_adr();

	enable_core_timer();
	reset_core_timer_absolute(MAX_ULONG);
	enable_interrupts_in_el1();

	init_frames();
	init_allocator();

	uart_send_string("Kernel Starts...\r\n");
	shell_loop();
}