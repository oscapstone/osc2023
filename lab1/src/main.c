#include "uart.h"
#include "utils.h"
#include "reboot.h"
#include "mailbox.h"

#define SIZE 128

void usage() {
    uart_writes("help\t: print this help memu\r\n");
    uart_writes("hello\t: print Hello World!\r\n");
    uart_writes("info\t: print hardware's information\r\n");
    uart_writes("reboot\t: reboot the device\r\n");
}

void hello_world() {
    uart_writes("Hello World!\r\n");
}

void not_found(char *s) {
    uart_writes(s);
    uart_writes(" command not found\r\n");
}

void reboot() {
    uart_writes("It is rebooting now...\r\n");
    reset(100);
}

void info() {
    get_board_revision();
    get_arm_memory();
}

int main()
{
    uart_init();
    uart_writes("\rLab1\n");
    while (1) {
        char buffer[SIZE];
        uart_writes("# ");
        uart_reads(buffer, SIZE);

        if (!str_cmp(buffer, "\0"))           uart_writes("\r\n");
        else if (!str_cmp(buffer, "help"))    usage();
        else if (!str_cmp(buffer, "hello"))   hello_world();
        else if (!str_cmp(buffer, "info"))    info(); 
        else if (!str_cmp(buffer, "reboot"))  reboot();
        else                                not_found(buffer);
    }

    return 0;
}