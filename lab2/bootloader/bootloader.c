#include "muart.h"

void bootloader(void) {
    mini_uart_init();
    mini_uart_puts("\r\nbootloader\r\n");
}