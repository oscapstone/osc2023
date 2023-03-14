#include "dtb.h"
#include "uart.h"
#include "file.h"
#include "my_stdint.h"

extern unsigned char __dtb_address;

fdt_header *dtb_address;

uint32_t swap_endian(uint32_t num)
{
    return num >> 24 | num << 24 | (num & 0xff00) << 8 | (num & 0xff0000) >> 8;
}


void fdt_init()
{
    uint32_t *tmp_pointer = (uint32_t *) &__dtb_address;
    // uint32_t *tmp_pointer = (uint32_t *) 0x7fff0;

    dtb_address = (fdt_header *) *tmp_pointer;
}


void fdt_traverse()
{
    uart_hex(swap_endian(dtb_address->magic));
}


// void fdt_traverse(device_node_callback_t callback)
// {
//     // take a look
    
// }