#include "dtb.h"
#include "uart.h"
#include "file.h"
#include "my_stdint.h"

extern unsigned char __dtb_address;

fdt_header *dtb_address;

void fdt_init()
{
    // dtb_address = (fdt_header *) &__dtb_address;
    uint32_t *tmp_pointer = (uint32_t *) &__dtb_address;
    // uint32_t *tmp_pointer = (uint32_t *) 0x7fff0;

    uart_puts("tmp_pointer :");
    uart_hex(tmp_pointer);
    uart_send('\n');

    dtb_address = (fdt_header *) *tmp_pointer;

    uart_puts("dtb_address :");
    uart_hex(dtb_address);
    uart_send('\n');
    uart_puts("total size :");
    uart_hex(dtb_address->totalsize);
    uart_send('\n');

    // readfile(dtb_address, dtb_address->totalsize);
    uart_puts("\nafter this\n");
}


void fdt_traverse()
{

    uart_puts("dtb_address :");
    uart_hex(dtb_address);
    uart_send('\n');

    // take a look
    uart_puts("magic :");
    uart_hex(dtb_address->magic);
    uart_send('\n');

    uart_puts("total size :");
    uart_hex((int) dtb_address->totalsize);
    uart_send('\n');

    uart_puts("right magic :");
    uart_hex(FTB_MAGIC);
    uart_send('\n');

    


    // readfile((char *) dtb_address, sizeof(fdt_header));
    // uart_send('\n');
}


// void fdt_traverse(device_node_callback_t callback)
// {
//     // take a look
    
// }