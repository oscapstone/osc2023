#include "uart.h"
#include "mbox.h"
#include "reset.h"
#include "initrd.h"
#include "string.h"

int i;
char c;
char buffer[100];

void input() {
	do {
            c = uart_getc();
            uart_send(c);
            buffer[i] = c;
            if (buffer[i] == 127) {
                buffer[i--] = 0;
                buffer[i--] = 0;
                uart_send('\b');
                uart_send(' ');
                uart_send('\b');
            }
            i++;
        } while (c != '\n');
}

void main()
{
    uart_init();
    heap_init();
    while(1) {
	i = 0;
        memset(buffer, sizeof(buffer));
	uart_puts("# ");
        input(); 
        uart_puts("\r");
        if (strncmp(buffer, "help", 4)==0) {
            uart_puts("help      : print this help menu\r\nhello     : print Hello World!\r\nreboot    : reboot the device\r\nmailbox   : show hardware info\r\nls        : list all file\r\ncat       : print file contents\r\nalloc     : allocate memory\r\n");
        } else if (strncmp(buffer, "hello", 5)==0) {
            uart_puts("Hello World!\r\n");
        } else if (strncmp(buffer, "mailbox", 7)==0 ){
		// Mailbox
	    get_board_revision();
    	    uart_puts("\r\nboard revision is ");
    	    uart_hex(mbox[5]);
    	    uart_puts("\r\n");

    	    get_arm_memory();
    	    uart_puts("ARM memory base address is ");
    	    uart_hex(mbox[5]);
    	    uart_puts("\r\n");
    	    uart_puts("ARM memory size is ");
    	    uart_hex(mbox[6]);
    	    uart_puts("\n\r");
	} else if (strncmp(buffer, "reboot", 6)==0) {
		uart_puts("\rRebooted!!!\n\r");
		reset(1);
	} else if (strncmp(buffer, "ls", 2)==0) {
		initrd_ls();
	} else if (strncmp(buffer, "cat", 3)==0) {
		i = 0;
		memset(buffer, sizeof(buffer));
		uart_puts("\rName:\n");
                input();       		
  		initrd_cat(buffer);
	} else if (strncmp(buffer, "alloc", 5)==0) {
		char *alloc = malloc(8);
		uart_puts("8 bytes allocated, starts from address: \n");
    		uart_hex((unsigned int)alloc);
    		uart_puts("\n");
	} else {
		uart_puts("Command not found\r\n");
	}
    }
}
