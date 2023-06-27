#ifndef __UART__
#define __UART__
#include <stddef.h>
#include <stdint.h>

#define BASE_ADDR			0x3f000000
#define AUX_ENABLES       	BASE_ADDR + 0x215004
#define AUX_MU_CNTL_REG   	BASE_ADDR + 0x215060
#define AUX_MU_IER_REG    	BASE_ADDR + 0x215044
#define AUX_MU_LCR_REG    	BASE_ADDR + 0x21504c
#define AUX_MU_MCR_REG    	BASE_ADDR + 0x215050
#define AUX_MU_BAUD_REG   	BASE_ADDR + 0x215068
#define AUX_MU_IIR_REG    	BASE_ADDR + 0x215048
#define AUX_MU_LSR_REG    	BASE_ADDR + 0x215054
#define AUX_MU_IO_REG     	BASE_ADDR + 0x215040
#define GPFSEL0             BASE_ADDR + 0x200000
#define GPFSEL1             BASE_ADDR + 0x200004
#define GPPUD               BASE_ADDR + 0x200094
#define GPPUDCLK0           BASE_ADDR + 0x200098
#define GPPUDCLK1           BASE_ADDR + 0x20009c

void uart_init();

size_t uart_write_1c(char*buf);
size_t uart_write(char* buf, size_t len);
size_t uart_readline(char* buf);
void uart_print(char* buf);
void uart_print_hex(uint64_t num, int len);
void newline();

void uart_interrupt_handler();
size_t uart_read_sync(char* buf, size_t len);
size_t uart_read_async(char* buf, size_t len);

#endif
#define uart_read uart_read_async