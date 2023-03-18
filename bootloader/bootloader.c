#include "utils.h"
#include "uart.h"
char *kernel = (char *)(0x80000);
extern char *_dtb;

void load_kernel(char *dtb_ptr)
{
    char buf[17] = {0};
    for (int i = 0; i < 16; i++) {
        //don't convert \r
        buf[i] = _uart_read();
    }
    unsigned long long kernel_len = hex2ull(buf);
    unsigned long long i, checksum = 0;
    for (i = 0; i < kernel_len; i++) {
        kernel[i] = _uart_read();
        if (i % 8 == 7) {
            checksum += *(unsigned long long *)(kernel + i - 7);
        }
    }
    unsigned remains = (kernel_len % 8);
    if (remains != 0) {
        checksum += ((*(unsigned long long *)(kernel + kernel_len - 8)) & (~(1 << remains))) << (8 - remains);
    }
    uart_write_string("kernel checksum with zero padding: ");
    uart_write_no_hex(checksum);
    uart_write_string("\n");
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
    //wait for python and test uart read function.
    char c;
    do {
        c = uart_read();
        uart_write(c);
    } while (c != 's');
    // uart_write_string("hello world!");
    load_kernel(dtb_ptr);
}