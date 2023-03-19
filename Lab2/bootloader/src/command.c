#include "uart.h"
#include "string.h"
#include "uart_boot.h"

void command_not_found (char * s) 
{
    uart_puts("Err: command ");
    /*uart_puts(s);*/
    uart_puts(" not found, try <help>\n");
}

void input_buffer_overflow_message ( char cmd[] )
{
    uart_puts("Follow command: \"");
    uart_puts(cmd);
    uart_puts("\"... is too long to process.\n");

    uart_puts("The maximum length of input is 64.");
}

void command_help ()
{
    uart_puts("help\t:  Print this help menu.\n");
    uart_puts("reboot\t:  Reboot the device.\n");
    uart_puts("loadimg\t:  Load the new kernel img.\n");
    uart_puts("clear\t:  Clear screen.\n");
    // uart_puts("timestamp:\tGet current timestamp.\n");

}



void command_reboot ()
{
    uart_puts("Start Rebooting...\n");

    *PM_WDOG = PM_PASSWORD | 0x20;
    *PM_RSTC = PM_PASSWORD | 100;
    
	while(1);
}



void command_clear (){
    uart_puts("\033[2J\033[H");
}




void command_loadimg (){
    load_img();
}
