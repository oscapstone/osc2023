#include "uart.h"
#include "load.h"
#include "string.h"

char* dtb_base;

void load() {
    char *kernel = (char *)(0x80000);
    int kn_ptr = 0;

    char size_str[50];
    int idx = 0;
    char c;
    while (1) {
        c = uart_get();
        if (c == '\n') {
            size_str[idx] = '\0';
            break;
        }
        size_str[idx] = c;
        idx++;
    }

    int size = atoi(size_str);
    uart_printf("Img size: %d\n", size);
    while (size--)
        kernel[kn_ptr++] = uart_get();

    void (*run)(char *) = (void *)kernel;
    run(dtb_base);
}