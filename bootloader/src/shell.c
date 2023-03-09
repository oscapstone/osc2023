#include "shell.h"
#include "my_string.h"
#include "uart.h"
#include "power.h"


void shell_init()
{
    uart_init();
    uart_puts("\n\n Hello from Raspi 3\n");
}

void shell_input(char *cmd)
{
    char c;

    uart_puts("\r# ");
    int idx = 0, end = 0;

    while ((c = uart_getc()) != '\n') {
        uart_send(c);
        cmd[idx++] = c;
        cmd[++end] = '\0';
    }

}

void shell_controller(char *cmd)
{

    uart_send('\n');
    if (!strcmp(cmd, ""))
        return;
    else if (!strcmp(cmd, "help")) {
        uart_puts("help      : print this help menu\n");
        uart_puts("hello     : print Hello World!\n");
        // uart_puts("timestamp: get current timestamp\n");
        uart_puts("reboot    : reboot the device\n");
    } else if (!strcmp(cmd, "hello")) {
        uart_puts("Hello World!\n");
    } else if (!strcmp(cmd, "reboot")) {
        reset();
    } else if (!strcmp(cmd, "shutdown")) {
        power_off();
    } else {
        uart_puts("shell: command not found\n");
    }
}