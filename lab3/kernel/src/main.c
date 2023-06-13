#include "uart.h"
#include "utils.h"
#include "dtb.h"
#include "timer.h"
#include "exception.h"

extern char* dtb_ptr;

// 可以用 asm volatile 把x30拿出來, 就不用用傳參的方式
void main(char* arg) { 
	dtb_ptr = arg; //get dtb from bootloader
	traverse_device_tree(dtb_ptr, dtb_callback_initramfs);
	shell_init();
	irqtask_list_init();
	timer_list_init();
	uart_interrupt_enable();
	el1_interrupt_enable(); //到main時,已經從el2->el1了, 所以要enable el1 interrupt.
	core_timer_enable();

	char cmd[100];
	cli_cmd_clear(cmd, 100);
	int status=0;
	while (1) {
		switch (status) {
			case 0:
				shell_input(cmd);
				status = 1;
				break;
			case 1:
				shell_command(cmd);
				status = 0;
				break;
		}
	}
}


                   