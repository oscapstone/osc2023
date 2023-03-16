#include "cpio/cpio.h"
#include "utils.h"
#include "peripherals/mini_uart.h"

char *initramfs_addr;

void set_initramfs_addr(uint32_t addr) {
    initramfs_addr = (char*)addr;
}

unsigned int hex_to_uint(char* c) {
    unsigned int ret = 0;
    for(int i = 0; i < 8; i ++, c ++) {
        // uart_send(*c);
        if(*c <= 'f' && *c >= 'a') {
            ret = (ret << 4) + *c - 'a' + 10;
        } else if(*c <= 'F' && *c >= 'A') {
            ret = (ret << 4) + *c - 'A' + 10;
        } else if(*c <= '9' && *c >= '0') {
            ret = (ret << 4) + *c - '0';
        } else {
            return 0;
        }
    }
    return ret;
}

void list_files() {
    char *tmp = (char *)initramfs_addr;
    int offset = sizeof(cpio_newc_header);
    for(;;) {
        unsigned int namesize = hex_to_uint(((cpio_newc_header*)tmp)->c_namesize);
        unsigned int filesize = hex_to_uint(((cpio_newc_header*)tmp)->c_filesize);

        if(strncmp(tmp + offset, "TRAILER!!!", 10) == 0) {
            break;
        }
        uart_send_n(tmp + offset, namesize);
        uart_send_string("\r\n");
        tmp += (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
        tmp += ((((filesize & 0x3) ^ 0x3) + 1) & 0x3) + filesize;
    }
}
unsigned int cat_file(const char *filename) {
    char *tmp = (char *)initramfs_addr;
    int offset = sizeof(cpio_newc_header);
    for(;;) {
        unsigned int namesize = hex_to_uint(((cpio_newc_header*)tmp)->c_namesize);
        unsigned int filesize = hex_to_uint(((cpio_newc_header*)tmp)->c_filesize);

        if(strncmp(tmp + offset, "TRAILER!!!", 10) == 0) {
            uart_send_string("file not found!\r\n");
            return 255;
        }
        if(strncmp(tmp + offset, filename, namesize) == 0) {
            uart_send_n(tmp + (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset, filesize);
            return 0;
        }
        // Since the (name + offset) size will be pad to align 4bytes
        tmp += (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
        // Since the filesize will be pad to align 4 bytes
        tmp += ((((filesize & 0x3) ^ 0x3) + 1) & 0x3) + filesize;
    }
}