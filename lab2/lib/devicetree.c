#include "muart.h"
#include "utils.h"
#include "devicetree.h"

void *DEVICETREE_ADDRESS = 0;

unsigned int to_little_endian(unsigned int num) {
    return (
        ((num >> 24) & 0x000000FF) | ((num >>  8) & 0x0000FF00) | 
        ((num <<  8) & 0x00FF0000) | ((num << 24) & 0xFF000000)
    );
}

void devicetree_get_address(void) {
    asm volatile("mov %0, x20" :"=r"(DEVICETREE_ADDRESS));

    char magic[4] = {0xd0, 0x0d, 0xfe, 0xed};
    if (strncmp((char*) DEVICETREE_ADDRESS, magic, 4) != 0) {
        mini_uart_puts("DeviceTree magic FAILED!\r\n");
    } else {
        mini_uart_puts("DeviceTree magic SUCCEED!\r\n");
    }
}

void fdt_traverse(void (*callback)(char*, char*, struct fdt_prop*)) {
    char magic[4] = {0xd0, 0x0d, 0xfe, 0xed};
    struct fdt_header *dt_header = DEVICETREE_ADDRESS;

    if (strncmp((char*) dt_header, magic, 4) != 0) {
        mini_uart_puts("DeviceTree magic FAILED!\r\n");
        return;
    }

    void *dt_struct_addr = DEVICETREE_ADDRESS + to_little_endian(dt_header->off_dt_struct);
    char *dt_string_addr = DEVICETREE_ADDRESS + to_little_endian(dt_header->off_dt_strings);

    unsigned int offset = 0;
    char *nodename = 0, *propname = 0;

    while (1) {
        unsigned int token = to_little_endian(*((unsigned int*) dt_struct_addr));

        if (token == FDT_BEGIN_NODE) {
            nodename = dt_struct_addr + 4;
            offset = 4 + strlen(nodename) + 1;
            offset = (offset % 4)? ((offset / 4) + 1) * 4: offset;
            dt_struct_addr += offset;
        } else if (token == FDT_END_NODE) {
            dt_struct_addr += 4;
        } else if (token == FDT_PROP) {
            struct fdt_prop *prop = (struct fdt_prop*)(dt_struct_addr + 4);
            offset = 4 + sizeof(struct fdt_prop) + to_little_endian(prop->len);
            offset = (offset % 4)? ((offset / 4) + 1) * 4: offset;
            dt_struct_addr += offset;
            propname = dt_string_addr + to_little_endian(prop->nameoff);
            callback(nodename, propname, prop);  
        } else if (token == FDT_NOP) {
            dt_struct_addr += 4;
        } else if (token == FDT_END) {
            dt_struct_addr += 4; break;
        } else {
            mini_uart_puts("token not matched\r\n");
            break;
        }
    }
}