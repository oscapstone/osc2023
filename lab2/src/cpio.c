#include "mini_uart.h"
#include "cpio.h"
#include "utils.h"
#include "devicetree.h"

void *DEVICETREE_CPIO_BASE;

void cpio_ls() {
    char *cpio_base;
    char *filename;
    char size_buf[9];
    size_buf[8] = '\0';
    int namesize, filesize, offset;
    struct cpio_newc_header *cpio_header;

    // Get the CPIO base address

    cpio_base = (char *) DEVICETREE_CPIO_BASE;
    // cpio_base = (char *) CPIO_QEMU_BASE;    // 0x8000000
    // cpio_base = (char *) CPIO_RPI_BASE;  // 0x20000000
    while(1){
    // Iterate over the CPIO archive
        // Get the current file header
        cpio_header = (struct cpio_newc_header *) cpio_base;

        // Check for magic
        if(strcmp(cpio_header->c_magic, CPIO_MAGIC) == 0){
            uart_puts("Error magic number!\r\n");
            return;
        }

        filename = (char *) cpio_header + HEADER_SIZE;
        
        // Check for end of archive
        if(strcmp(filename, CPIO_FOOTER) == 0){
            // uart_puts("TRAILER!!!\r\n");
            break;
        }

        uart_puts(filename);
        uart_puts("\r\n");

        memcpy(size_buf,cpio_header->c_namesize,8);
        namesize = strtol(size_buf, 0, 16);

        memcpy(size_buf,cpio_header->c_filesize,8);
        filesize = strtol(size_buf, 0, 16);

        // Align to 4 byte
        offset = ((HEADER_SIZE + namesize + 3) / 4) * 4;
        offset += ((filesize + 3) / 4) * 4;

        cpio_base += offset;
    }
        
    
}

void cpio_cat(){
    char *cpio_base;
    char *filename;
    char size_buf[9];
    size_buf[8] = '\0';
    char buffer[BUFFER_SIZE];
    int namesize, filesize, offset;
    struct cpio_newc_header *cpio_header;

    // Get the CPIO base address

    cpio_base = (char *) DEVICETREE_CPIO_BASE;
    // cpio_base = (char *) CPIO_QEMU_BASE;    // 0x8000000
    // cpio_base = (char *) CPIO_RPI_BASE;  // 0x20000000


    // Get filename
    uart_puts("Filename: ");
    uart_gets(buffer);

    while(1){
        cpio_header = (struct cpio_newc_header *) cpio_base;

        if(strcmp(cpio_header->c_magic, CPIO_MAGIC) == 0){
            uart_puts("Error magic number!\r\n");
            return;
        }

        filename = (char *) cpio_header + HEADER_SIZE;

        if(strcmp(filename, CPIO_FOOTER) == 0){
            break;
        }

        memcpy(size_buf,cpio_header->c_namesize,8);
        namesize = strtol(size_buf, 0, 16);

        memcpy(size_buf,cpio_header->c_filesize,8);
        filesize = strtol(size_buf, 0, 16);

        offset = ((HEADER_SIZE + namesize + 3) / 4) * 4;

        if(strcmp(buffer,filename) == 0){
            char *content = (char *) cpio_header + offset;
            for (int i = 0; i < filesize; i++) {
                if(content[i] == '\n'){
                    uart_puts("\r\n");
                }
                else{
                    uart_send(content[i]);
                }
            }
            uart_puts("\r\n");
            return;
        }

        offset += ((filesize + 3) / 4) * 4;
        cpio_base += offset;
    }
    uart_puts("Sorry, there's no file called \"");
    uart_puts(buffer);
    uart_puts("\"\r\n");
    return;
}

void initramfs_callback(char *nodename, char *propname, struct fdt_prop* prop) {
    if (strncmp(nodename, "chosen", 7) == 0 && 
        strncmp(propname, "linux,initrd-start", 19) == 0) {
        DEVICETREE_CPIO_BASE = (void*)((long unsigned int) htonl(*((unsigned int*)(prop + 1))));
        uart_puts("Get DEVICETREE_CPIO_BASE ! \r\n");
    }
}