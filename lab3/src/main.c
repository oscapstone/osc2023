#include "mini_uart.h"
#include "shell.h"
#include "cpio.h"
#include "utils.h"
#include "peripherals/mailbox.h"
#include "dtb.h"
#define MAX_CMD 512

enum stat {
	read,
	parse
};

void kernel_main(void *dtb)
{
	unsigned int *dest;
	uart_init();
	uart_puts("\n");

	// Welcome();
	uart_puts("\n");
	uart_puts("Please type: \n");

	enum stat s = read;
	char *cmd[MAX_CMD];
	// initrd_list();
	dest = load_prog("usr.img");
	// uart_puts("\nProgram is loaded into address: ");
	exec_prog(dest);

	// buf_clear(cmd);
	while (1) {
		// initrd_list();
		if(s == read){
			uart_puts("# ");
			// TO test inst,  quote this line
			shell_input(cmd);
			s = parse;
		} else {
			// TO test inst,  modify this line
			parse_cmd(cmd, dtb);
			// parse_cmd("ls");
			s = read;
		}
	}
}

void Welcome(){
	uart_puts("  ____ _____ _____ _____ _    _   _    ___  ____   ____ ____   ___ ____  _____ \n");
	uart_puts(" / ___|_   _| ____|  ___/ \\  | \\ | |  / _ \\/ ___| / ___|___ \\ / _ \\___ \\|___ / \n");
	uart_puts(" \\___ \\ | | |  _| | |_ / _ \\ |  \\| | | | | \\___ \\| |     __) | | | |__) | |_ \\ \n");
	uart_puts("  ___) || | | |___|  _/ ___ \\| |\\  | | |_| |___) | |___ / __/| |_| / __/ ___) |\n");
	uart_puts(" |____/ |_| |_____|_|/_/   \\_\\_| \\_|  \\___/|____/ \\____|_____|\\___/_____|____/ \n");
}
