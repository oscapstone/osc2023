#include "mini_uart.h"
#include "load_kernel.h"

void bootloader_main(void)
{
    uart_init();

    uart_send_string("Relocatting ...\n");
    char *from_dest = (char *)0x80000;
    char *to_dest = (char *)0x60000;
    relocate((char *)from_dest, (char *)to_dest);

    char *dest = (char *)0x80000;
    load_kernel((char *)dest);
}
