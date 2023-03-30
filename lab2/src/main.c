#include "shell.h"
#include "dtb.h"
#include "uart.h"
#include "heap.h"
#include "my_str.h"

extern char* dtb_ptr;
// extern void* CPIO_DEFAULT_PLACE;
void main(char *arg)
{
	shell_init();
	// uart_printf("%x\n", &arg);
	char input_buffer[CMD_MAX_LEN];
	dtb_ptr = arg;
	traverse_device_tree(dtb_ptr, dtb_callback_initramfs);
	uart_printf("loading dtb from: 0x%x\n", arg);
	while(1) {
		cli_cmd_clear(input_buffer, CMD_MAX_LEN);
		
		shell_input(input_buffer);
		shell_controller(input_buffer);
	}
}