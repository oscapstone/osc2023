#include "cpio.h"
#include "mini_uart.h"
#include "str_tool.h"
#include "devicetree.h"

void *DEVICETREE_CPIO_BASE = 0;

void cpio_list(void) {
    unsigned int headerlen = sizeof(struct cpio_newc_header);
    struct cpio_newc_header *current = (void*) CPIO_QEMU_BASE;
    //struct cpio_newc_header *current = (void*) CPIO_RPI_BASE;


    while (1) {
        char *filename = ((void*) current) + headerlen;
        

        if (strncmp(current->c_magic, CPIO_MAGIC, 6) != 0) {
            uart_puts(CPIO_MAGIC);
            uart_puts(current->c_magic);
            uart_puts("invalid magic\r\n"); 
            break;
        }

        if (strncmp(filename, CPIO_FOOTER, 10) == 0) {
            break;
        }

        uart_puts(filename);
        uart_puts("\r\n");

        unsigned int namesize = atoi(current->c_namesize, 8);
        unsigned int filesize = atoi(current->c_filesize, 8);
        unsigned int offset   = headerlen + namesize;

        offset   = (offset % 4)?   ((offset / 4) + 1) * 4  : offset;
        filesize = (filesize % 4)? ((filesize / 4) + 1) * 4: filesize;

        current = ((void*) current) + offset + filesize;
    }
}

void cpio_concatenate(void) {
    unsigned int headerlen = sizeof(struct cpio_newc_header);
    struct cpio_newc_header *current = (void*) CPIO_QEMU_BASE;
    //struct cpio_newc_header *current = (void*) CPIO_RPI_BASE;

    
    char buffer[BUFSIZE];

    for (int i = 0; i < 1; i++) {
        uart_puts("Filename: ");
        uart_gets(buffer, BUFSIZE);
    }

    while (1) {
        char *filename = ((void*) current) + headerlen;

        if (strncmp(current->c_magic, CPIO_MAGIC, 6) != 0) {
            uart_puts("invalid magic\r\n"); break;
        }

        if (strncmp(filename, CPIO_FOOTER, 10) == 0) {
            break;
        }

        unsigned int filesize = atoi(current->c_filesize, 8);
        unsigned int namesize = atoi(current->c_namesize, 8);
        unsigned int offset   = headerlen + namesize;

        offset = (offset % 4)? ((offset / 4) + 1) * 4: offset;

        if (strncmp(buffer, filename, namesize) == 0) {
            char *content = ((void*) current) + offset;
            for (int i = 0; i < filesize; i++) {
                uart_putc(content[i]);
            }
            if (content[filesize - 1] != '\n') {
                uart_puts("\r\n");
            }
        }

        filesize = (filesize % 4)? ((filesize / 4) + 1) * 4: filesize;
        current  = ((void*) current) + offset + filesize;
    }
}

void initramfs_callback(char *nodename, char *propname, struct fdt_prop* prop) {
    if (strncmp(nodename, "chosen", 7) == 0 && 
        strncmp(propname, "linux,initrd-start", 19) == 0) {
        DEVICETREE_CPIO_BASE = (void*)((long unsigned int) to_little_endian(*((unsigned int*)(prop + 1))));
    }
}