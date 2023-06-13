#include "mini_uart.h"
#include "utils_c.h"
#include "utils_assembly.h"
#include <stddef.h>
#define BUFFER_MAX_SIZE 256u

extern char *_dtb; // 定義在bootloader的start.s中
void load_img()
{
    char *const kernel_addr = (char *)0x80000;
    uart_send_string("Please sent the kernel image size:");
    char buffer[BUFFER_MAX_SIZE];
    // read_command(buffer);
    size_t index = 0;
    while (1)
    {
        buffer[index] = uart_recv();
        uart_send(buffer[index]);
        if (buffer[index] == '\n')
        {
            break;
        }
        index++;
    }
    buffer[index + 1] = '\0';
    //???
    utils_newline2end(buffer);
    uart_send('\r');

    unsigned int img_size = utils_str2uint_dec(buffer);
    uart_send_string("Start to load the kernel image... \n"); // 會多送一個\r

    unsigned char *current = kernel_addr;
    while (img_size--)
    {
        *current = uart_recv_raw();
        current++;
        // uart_send('.');
    }
    uart_send_string("finishing receiving\n");
    uart_send_string("loading...\n");

    // 轉去執行kernel.img,用function pointer的方式
    // 並且_dtb原先在bootloader's start.s中從x0拿到了dtb(device tree blob)的位置,
    // 所以_dtb是指向dtb的指標，
    //  branchAddr(kernel_addr);
    ((void (*)(char *))kernel_addr)(_dtb); //(void (*)(char *)) is a function pointer type
}

void bootloader_main(void)
{
    uart_init();
    uart_send_string("In bootloader_main!\n");
    load_img();
}