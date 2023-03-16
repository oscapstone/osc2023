#include "uart.h"
#include "string.h"
#include "load.h"

/* shell */
void shell(void) {
    uart_puts("! Welcome Bootloader !\n");

    char command[32];
    int idx = 0;

    while (1) {
        idx = 0;
        uart_puts("$ ");
        while (1) {
            command[idx] = uart_getc();
            if (command[idx] == '\n') {
                command[idx] = '\0';
                break;
            }
            idx++;
        }

        // Lab 1
        if (strcmp("hello", command) == 0) {
            uart_puts("\n");
            uart_puts("Hello Bootloader!\n");
        }
        // Lab 2
        else if (strcmp("load", command) == 0) {
            uart_puts("\n");
            uart_puts("loading...\r\n");
            load();
        }
        else {
            uart_puts("\n");
            uart_puts("unknown\n");
        }
        
    }
        
}

