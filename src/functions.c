#include "mini_uart.h"
#include "mbox.h"
#include "utils.h"
#define     NUM_FUNCS   4

void help() {
    char *help_message = "help      : print this help menu\r\nhello     : print Hello World!\r\nreboot    : reboot the device\r\nsystem    : get system information";
    uart_send_string(help_message);
}

void hello() {
    uart_send_string("Hello World!");
}

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void reboot() {                 
    put32(PM_RSTC, PM_PASSWORD | 0x20);  
    put32(PM_WDOG, PM_PASSWORD | 100); 
    uart_send_string("Rebooting... \r\n");
    while(1);
}

char COMMANDS[NUM_FUNCS][64] = {"help", "hello", "reboot", "system"};

int function_selector(char* a) {
    for(int i = 0 ; i < NUM_FUNCS ; i++) {
        int j = 0;
        while(*(COMMANDS[i] + j) != '\0' && *(a + j) != '\0') {
            if(*(COMMANDS[i] + j) != *(a + j)) break;
            j++;
        }
        if(*(COMMANDS[i] + j) == '\0') return i;
    }
    return -1;
}

void system() {
    get_board_revision();
    get_memory_info();
}
