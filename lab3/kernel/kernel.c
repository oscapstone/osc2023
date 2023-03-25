#include "mini_uart.h"
#include "shell.h"
#include "device_tree.h"
#include "exception.h"
#include "timer.h"

void kernel_main(void)
{
	set_exception_vector_table();
	uart_init();
	get_device_tree_adr();
	enable_core_timer();
	reset_core_timer_in_cycle((unsigned int)0xffffffff);
	uart_send_string("Kernel Starts...\r\n");
	shell_loop();
}