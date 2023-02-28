#include <mini_uart.h>
#include <string.h>
#include <builtin.h>

#define BUFSIZE 0x100
char shell_buf[BUFSIZE];

void shell_interact(void){
    uart_send_string("# ");
    unsigned int cnt = uart_recv_line(shell_buf, BUFSIZE);
    uart_send_string("\r\n");
    if (!strcmp("help", shell_buf))
        _help();
    else if (!strcmp("hello", shell_buf)) 
        _hello();
    else if (!strcmp("reboot", shell_buf))
        _reboot();
    else if (!strcmp("hwinfo", shell_buf))
        _hwinfo();
    else {
        _echo(shell_buf);
        if(cnt)
            uart_send_string("\r\n");
    }
}

void kernel_main(void){
    uart_init();
    uart_send_string("Rpi3 start running!\r\n");

    while(1)
        shell_interact();
}