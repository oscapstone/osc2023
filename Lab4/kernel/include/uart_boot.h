#ifndef _UART_BOOT_H
#define _UART_BOOT_H
#include "peripherals/rpi_uart.h"

#define KERNEL_ADDR ((char*)0x80000)
#define TEMP_ADDR   ((char*)0x300000)

void loading();
void load_new_kernel();

#endif /*_UART_BOOT_H */