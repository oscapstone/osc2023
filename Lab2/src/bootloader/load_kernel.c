#include "utils.h"
#include "mini_uart.h"
#include "peripherals/mini_uart.h"
#include "stdlib.h"

extern int __loader_size;
extern void *_dtb_ptr;

void load_kernel(char *dest)
{
    char *const kernel_addr = dest;

    int size = 0;

    uart_send_string("Waiting for kernel8.img ...\n");

    char start[5] = "Start";

    for (int i = 0; i < 5; i++)
    {
        if (uart_recv() != start[i])
            i = 0;
    }

    uart_send_string("Start transmitting ...\n");

    size = uart_recv();
    size |= uart_recv() << 8;
    size |= uart_recv() << 16;
    size |= uart_recv() << 24;

    printf("Size of kernel is = %d bytes\n", size);

    // read the kernel
    while (size--)
    {
        while (1)
        {
            if (get32(AUX_MU_LSR_REG) & 0x01)
                break;
        }
        *dest++ = get32(AUX_MU_IO_REG);
    }

    uart_send_string("End transmitting ...\n");

    // goto *kernel_addr;

    // ((void (*)(char *))kernel_addr)(_dtb_ptr);

    // restore arguments and jump to the new kernel.
    asm volatile(
        "mov x0, x10;"
        "mov x1, x11;"
        "mov x2, x12;"
        "mov x3, x13;"
        // we must force an absolute address to branch to
        "mov x30, 0x80000; ret");
}

void relocate(char *from_dest, char *to_dest)
{
    long long size = (long long)&__loader_size;

    while (size--)
    {
        *to_dest++ = *from_dest++;
    }

    char *redicrect = __builtin_return_address(0) + (to_dest - from_dest);

    goto *redicrect;
}