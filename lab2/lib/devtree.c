#include "devtree.h"
#include "mini_uart.h"
#include "string.h"

void *DEVTREE_ADDRESS = 0;

unsigned int to_lendian(unsigned int n) {
    return ((n>>24)&0x000000FF) |
           ((n>>8) &0x0000FF00) |
           ((n<<8) &0x00FF0000) |
           ((n<<24)&0xFF000000) ;
}

void devtree_getaddr() {
    
    asm volatile("MOV %0, x20" :  "=r"(DEVTREE_ADDRESS));

    char magic[4] = {0xd0, 0x0d, 0xfe, 0xed};
    if(stringncmp((char*)DEVTREE_ADDRESS, magic, 4) != 0) {
        uart_send_string("magic failed\n");
    } else {
        uart_send_string("devtree magic succeed\n");
    }

}

void fdt_traverse( void (*callback)(char *, char *, struct fdt_prop *) ) {
    
    char magic[4] = {0xd0, 0x0d, 0xfe, 0xed};
    struct fdt_header *devtree_header = DEVTREE_ADDRESS;

    if(stringncmp((char*)devtree_header, magic, 4) != 0) {
        uart_send_string("devtree magic failed\n");
        return;
    }

    void *dt_struct_addr = DEVTREE_ADDRESS + to_lendian(devtree_header->off_dt_struct);
    char *dt_string_addr = DEVTREE_ADDRESS + to_lendian(devtree_header->off_dt_strings);
    
    char *node_name;
    char *prop_name;
    unsigned int token;
    unsigned int off;

    while (1) {

        token = to_lendian(*((unsigned int *)dt_struct_addr));

        if (token == FDT_BEGIN_NODE) {

            node_name = dt_struct_addr + 4;
            off = 4 + strlen(node_name) + 1;
            
            if (off%4 != 0)
                off = ((off/4)+1)*4;
            
            dt_struct_addr += off;

        } 
        else if (token == FDT_END_NODE) {
            dt_struct_addr += 4;
        }
        else if (token == FDT_PROP) {

            struct fdt_prop *prop = (struct fdt_prop*)(dt_struct_addr + 4);

            off = 4 + 8 + to_lendian(prop->len);
            if (off%4 != 0)
                off = ((off/4)+1)*4;

            dt_struct_addr += off;
            
            prop_name = dt_string_addr + to_lendian(prop->nameoff);

            callback(node_name, prop_name, prop);

        }
        else if (token == FDT_NOP) {
            dt_struct_addr += 4;
        }
        else if (token == FDT_END) {
            dt_struct_addr += 4;
            break;
        }
        else {
            uart_send_string("TOKEN NOT MATCHED\n");
            break;
        }

    }

}