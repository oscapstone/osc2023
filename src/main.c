#include <mini_uart.h>
#include <string.h>
#include <BCM.h>
// #include <rpi3.h>

#define BUFSIZE 0x100
char shell_buf[BUFSIZE];

void kernel_main(void){
    uart_init();
    uart_send_string("Hello, world!\r\n");

    while(1){
        uart_send_string("# ");
        uart_recv_line(shell_buf, BUFSIZE);
        uart_send_string("\r\n");
        if (!strcmp("help", shell_buf)) {
            uart_send_string(
                "help\t: "   "print this help menu" "\r\n"
                "hello\t: "  "print Hello World!"   "\r\n"
                "reboot\t: " "reboot the device" "\r\n"
            );
        }
        else if (!strcmp("hello", shell_buf)) 
            uart_send_string("Hello World!\r\n");
        else if (!strcmp("reboot", shell_buf)) {
            uart_send_string("Reboot!\r\n");
            reset(10);
        }
        else {
            uart_send_string(shell_buf);
            uart_send_string("\r\n");
        }
    }
}