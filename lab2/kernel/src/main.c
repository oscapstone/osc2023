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
	uart_init();
	uart_puts("\n");
	void *memory = (void *)0x120000;
    char *mac = simple_malloc(&memory, 8);
    uart_puts("8 bytes allocated, starts from: \n");
    uart_hex((unsigned int)mac);
    uart_puts("\n");
    mac = simple_malloc(&memory, 34);
    uart_puts("34 bytes  bytes allocated, starts from: \n");
    uart_hex((unsigned int)mac);
    uart_puts("\n");
    mac = simple_malloc(&memory, 3);
    uart_puts("3 bytes  bytes allocated, starts from: \n");
    uart_hex((unsigned int)mac);
    uart_puts("\n");

	uart_puts("initrd before callback:");
	uart_hex(get_initramfs());
	uart_puts("\nfind dtb from ");
	uart_hex(dtb);
	find_dtb(dtb, "linux,initrd-start", 18, &callback_initramfs);

	uart_puts("\ninitrd after callback:");
	uart_hex(get_initramfs());
	uart_puts("\n");

	Welcome();
	uart_puts("\n");
	uart_puts("Please type: \n");

	enum stat s = read;
	char *cmd[MAX_CMD];
	buf_clear(cmd);
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