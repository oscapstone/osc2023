#include "bcm2835/uart.h"

extern char __kernel[];

static inline unsigned int read_kernel_size() {
    unsigned int size = 0;
    for (int i = 0; i < 4; ++i) {
        size = size | ((unsigned int)uart_recvraw() << (i * 8));
    }
    return size;
}

static inline void _load_kernel(unsigned int size) {
    char * addr = __kernel;
    while (size--) {
        *(addr++) = uart_recvraw();
    }
}

/*
 * Load kernel from mini UART. The size limit of the kernel image is 2^32 - 1
 * In this protocal, the first 4 bytes (little endian) represent the size of
 * the kernel image, and follow the image transmission.
 */

void load_kernel() {
    uart_init();
    unsigned int size = read_kernel_size();
    uart_puts("[*] kernel size: ");
    uart_pu32h(size);
    uart_send('\n');
    uart_puts("[*] loading kernel ...\n");
    _load_kernel(size);
    uart_puts("[*] kernel is loaded at ");
    uart_pu32h((unsigned int)(long long int)(__kernel));
    uart_send('\n');
}
