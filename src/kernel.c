#include "mini_uart.h"
#include "functions.h"
#include "mbox.h"

void kernel_main(void)
{
	uart_init();
	while(1) {
		uart_send_string("# ");
		char buf[256];
		uart_recv_string(buf);

		int selected_function = function_selector(buf);
        if(selected_function == -1) {
            uart_send_string("Unknown command.");
        }
        else {
            switch (selected_function){
                case 0:
                    help();
                    break;
                case 1:
                    hello();
                    break;
                case 2:
                    reboot();
                    break;
				case 3:
					system();
					break;
                default: 
                    break;
            }
        }
        uart_send_string("\r\n");
	}
}
