#include "devicetree.h"
#include "mini_uart.h"
#include "utils.h"

char *device_tree_ptr = 0;

// for debug
void hex_to_str(unsigned char ch, char *buf) {
    const char *hex = "0123456789abcdef";
    buf[0] = hex[ch >> 4];
    buf[1] = hex[ch & 0xf];
}

// for debug
void bytes_to_hex_str(const unsigned char *bytes, int len, char *buf) {
    int i;
    for (i = 0; i < len; i++) {
        hex_to_str(bytes[i], buf + 2*i);
    }
    buf[2*len] = '\0';
}

void get_devicetree_ptr(){
    asm volatile("mov %0, x20" :"=r"(device_tree_ptr));

    // char buf[9];
    // bytes_to_hex_str((unsigned char *)magic, 4, buf);
    // uart_puts(buf);
    // uart_puts("\r\n");


    if (strncmp(device_tree_ptr, "\xd0\x0d\xfe\xed", 4) != 0) {
        uart_puts("[DeviceTree] Wrong magic!\r\n");
    } else {
        uart_puts("[DeviceTree] Right magic!\r\n");
    }
    

}

void fdt_traverse(void (*callback)(char*, char*, struct fdt_prop*)) {
    struct fdt_header *header = (struct fdt_header*)device_tree_ptr;

    // Verify the header magic number
    if (strncmp(device_tree_ptr, "\xd0\x0d\xfe\xed", 4) != 0){
        uart_puts("[DeviceTree] Wrong magic!\r\n");
        return;
    }

    void *dt_struct_addr = device_tree_ptr + htonl(header->off_dt_struct);
    char *dt_string_addr = device_tree_ptr + htonl(header->off_dt_strings);

    unsigned int offset = 0;
    char *nodename = 0, *propname = 0;

    while (1) {
        unsigned int token = htonl(*((unsigned int*) dt_struct_addr));

        if (token == FDT_BEGIN_NODE) {
            nodename = dt_struct_addr + 4;
            offset = 4 + strlen(nodename) + 1;
            offset = ((offset + 3) / 4) * 4;
            dt_struct_addr += offset;
        } else if (token == FDT_END_NODE) {
            dt_struct_addr += 4;
        } else if (token == FDT_PROP) {
            struct fdt_prop *prop = (struct fdt_prop*)(dt_struct_addr + 4);
            offset = 4 + sizeof(struct fdt_prop) + htonl(prop->len);
            offset = ((offset + 3) / 4) * 4;
            dt_struct_addr += offset;
            propname = dt_string_addr + htonl(prop->nameoff);
            callback(nodename, propname, prop);  
        } else if (token == FDT_NOP) {
            dt_struct_addr += 4;
        } else if (token == FDT_END) {
            dt_struct_addr += 4; break;
        } else {
            uart_puts("token not matched\r\n");
            break;
        }
    }

    /*
        token       : unsigned int
        nodename    : char *
    */
    
    /*
        token       : unsigned int
        fdt_prop    : struct fdt_prop
        prop
    */

    /*
    • (optionally) any number of FDT_NOP tokens 
    • FDT_BEGIN_NODE token 
        – The node’s name as a null-terminated string 
        – [zeroed padding bytes to align to a 4-byte boundary] 
    • For each property of the node: 
        – (optionally) any number of FDT_NOP tokens 
        – FDT_PROP token 
            ∗ property information as given in Section 5.4.1 
            ∗ [zeroed padding bytes to align to a 4-byte boundary] 
    • Representations of all child nodes in this format 
    • (optionally) any number of FDT_NOP tokens 
    • FDT_END_NODE token
    */
}