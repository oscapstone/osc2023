#include "uart.h"
#include "mbox.h"
#include "reset.h"

int strcmp(const char *s1, const char *s2)
{
  while (*s1 != '\0' && *s2 != '\0')
    {
	    if (*s1++ != *s2++)
		    return 1;
	    if (*s1 == '\0')
		    return 0;
    }
  return 0;
}

void* memset(char *buffer, int size)
{
    for (int i = 0; i < size; i++)
    {
	    buffer[i] = '\0';
    }
}

void main()
{
    char buffer[100];
    char c;

    uart_init();

    while(1) {
	int i = 0;
        memset(buffer, sizeof(buffer));
        do {
            c = uart_getc();
            uart_send(c);
            buffer[i++] = c;
        } while (c != '\n');
        uart_puts("\r");
        if (strcmp(buffer, "help")==0) {
            uart_puts("help      : print this help menu\r\nhello     : print Hello World!\r\nreboot    : reboot the device\r\n");
        } else if (strcmp(buffer, "hello")==0) {
            uart_puts("Hello World!\r\n");
        } else if (strcmp(buffer, "mailbox")==0 ){
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
	} else if (strcmp(buffer, "reboot")==0) {
		uart_puts("\r");
		reset(1);
	}
	 
        	
    }
}
