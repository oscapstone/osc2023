#include "mini_uart.h"
#include "shell.h"


int main(){

    uart_init();
    /////////
    uart_display("Hello!\r\n");
    uart_display("****FOUR FUNCTIONS****\r\n");
    uart_display("******help******\r\n");
    uart_display("******hello******\r\n");
    uart_display("******hw_info******\r\n");
    uart_display("******reboot******\r\n");
    /////////
    shell_start();

    return 0;
}
