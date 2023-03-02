#include "muart.h"
#include "utils.h"
#include "mailbox.h"

#define SIZE 64

void usage(void) {
    mini_uart_puts("help\t: print this help memu\n");
    mini_uart_puts("info\t: print hardware's information\n");
    mini_uart_puts("hello\t: print Hello World!\n");
    mini_uart_puts("reboot\t: reboot the device\n");
}

void info(void) {
    get_board_revision();
    get_arm_memory();
}

void hello(void) {
    mini_uart_puts("Hello World!\n");
}

void reboot(void) {
    
}

void message(void) {
    mini_uart_puts("command not found\n");
}

int main(void) {
    mini_uart_init();

    while (1) {
        char buffer[SIZE];
        mini_uart_puts("# ");
        mini_uart_gets(buffer, SIZE);
        
        if (strcmp(buffer, "help") == 0) {
            usage();
        } else if (strcmp(buffer, "info") == 0) {
            info();
        } else if (strcmp(buffer, "hello") == 0) {
            hello();
        } else if (strcmp(buffer, "reboot") == 0) {
            reboot();
        } else {
            message();
        }
    }

    return 0;
}