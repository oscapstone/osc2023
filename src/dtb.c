#include "dtb.h"
#include "uart.h"
#include "file.h"
#include "my_stdint.h"
#include "my_string.h"
#include "initrd.h"

extern unsigned char __dtb_address;

fdt_header *dtb_address;

uint32_t swap_endian(uint32_t num)
{
    return num >> 24 | num << 24 | (num & 0x0000ff00) << 8 | (num & 0x00ff0000) >> 8;
}

void fdt_init()
{
    uint32_t *tmp_pointer = (uint32_t *) &__dtb_address;
    // uint32_t *tmp_pointer = (uint32_t *) 0x7fff0;

    dtb_address = (fdt_header *) *tmp_pointer;
}

int len_str(char *str)
{
    int count = 0;
    while (*str != '\0') {
        count++;
        str++;
    }
    return count;
}

void fdt_traverse(void (*callback)(fdt_prop *, char *, char *))
{
    if (swap_endian(dtb_address->magic) != FDT_MAGIC)
        return;

    uint32_t *struct_sp = (uint32_t *) ((char *)dtb_address + swap_endian(dtb_address->off_dt_struct));
    char *string_sp = (char *) ((uint32_t) dtb_address + swap_endian(dtb_address->off_dt_strings));
    
    char *node_name;

    while (1) {
        uint32_t token = swap_endian(*struct_sp);
        if (token == FDT_BEGIN_NODE) {
            fdt_node_header *node = (fdt_node_header *) struct_sp;
            node_name = node->name;
            struct_sp += ALIGN(len_str(node->name), 4) / 4;
        } else if (token == FDT_PROP) {
            fdt_prop* prop = (fdt_prop*)(struct_sp + 1);
            struct_sp += (sizeof(fdt_prop)+ALIGN(swap_endian(prop->len), 4))/4;
            char *property_name = string_sp+swap_endian(prop->nameoff);
            
            callback(prop, node_name, property_name);
            
        } else if (token == FDT_END_NODE || token == FDT_NOP) {

        } else if (token == FDT_END) {
            break;
        }
        struct_sp++;
    }
}


void initramfs_callback(fdt_prop *prop, char *node_name, char *property_name)
{
    if (!strcmp(node_name, "chosen") && !strcmp(property_name, "linux,initrd-start")) {
        uint32_t load_addr = *((uint32_t *)(prop + 1));
        cpio_base = swap_endian(load_addr);
        uart_puts("cpio_base: ");
        uart_hex(cpio_base);
        uart_send('\n');
    }
}