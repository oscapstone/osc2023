#include "utils.h"
char *kernel = (char *)(0x80000);
extern char *_dtb;

void load_kernel(char *dtb_ptr)
{
    //unsigned long long kernel_len = uart_read_hex_ull();
    char buf[17] = {0};
    for (int i = 0; i < 16; i++) {
        //don't convert \r
        buf[i] = _uart_read();
        //uart_write('.');
    }
    unsigned long long kernel_len = hex2ull(buf);
    unsigned long long i;
    for (i = 0; i < kernel_len; i++) {
        kernel[i] = _uart_read();
        //uart_write('.');
    }
    uart_write_string("kernel well received!\n");
    ((void (*)(char *))kernel)(dtb_ptr);
}

void main(char *dtb_ptr)
{
    uart_init();
    uart_write_string("device tree address: ");
    dump_hex(&dtb_ptr, 8);
    uart_write_string("\n");
    uart_write_string("device tree first 8 bytes: ");
    dump_hex(dtb_ptr, 8);
    uart_write_string("\n");

    char c;
    do {
        c = uart_read();
        uart_write(c);
    } while (c != 's');
    // uart_write_string("hello world!");
    load_kernel(dtb_ptr);
}