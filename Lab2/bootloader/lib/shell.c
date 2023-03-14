#include "uart.h"
#include "string.h"

/* shell */
void shell(void) {
    uart_puts("! Welcome Lab1 !\n");

    char command[32];
    int idx = 0;

    while (1) {
        idx = 0;
        uart_puts("$ ");
        while (1) {
            command[idx] = uart_getc();
            uart_send(command[idx]);
            if (command[idx] == '\n') {
                command[idx] = '\0';
                break;
            }
            idx++;
        }

        // Lab 1
        if (strcmp("hello", command) == 0) {
            uart_puts("\n");
            uart_puts("Hello World!\n");
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

