#include "mini_uart.h"
#include "cpio.h"
#include "utils.h"
#include "devicetree.h"

void *DEVICETREE_CPIO_BASE;

// for cat, exec
char target_name[BUFFER_SIZE];
int file_found = 0;


void cpio_parse(void (*callback)(char *, int, char *)){
    char *cpio_base;
    char *filename;
    char size_buf[9];
    size_buf[8] = '\0';
    int namesize, filesize, offset;
    struct cpio_newc_header *cpio_header;

    // Get the CPIO base address
    cpio_base = (char *) DEVICETREE_CPIO_BASE;

    while(1){
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
            break;
        }
        
        memcpy(size_buf,cpio_header->c_namesize,8);
        namesize = strtol(size_buf, 0, 16);

        memcpy(size_buf,cpio_header->c_filesize,8);
        filesize = strtol(size_buf, 0, 16);

        offset = ((HEADER_SIZE + namesize + 3) / 4) * 4;

        // Call the callback function with the file information
        callback(filename, filesize, (char *) cpio_header + offset);

        offset += ((filesize + 3) / 4) * 4;
        cpio_base += offset;
    }

    return;
}

void cpio_ls_callback(char *filename, int filesize, char *content) {

    uart_puts(filename);
    uart_puts("\r\n");

}

void cpio_cat_callback(char *filename, int filesize, char *content) {

    if(strcmp(target_name, filename) == 0) {
        for (int i = 0; i < filesize; i++) {
            if(content[i] == '\n'){
                uart_puts("\r\n");
            }
            else{
                uart_send(content[i]);
            }
        }
        uart_puts("\r\n");
        file_found += 1;
    }
}

void cpio_exec_callback(char *filename, int filesize, char *content) {

    if(strcmp(target_name, filename) == 0) {
        char * location = content;
        char *sp = location + 0x2000;
        asm volatile( "msr     elr_el1, %0" :: "r" (location) );// Exception return address
        asm volatile( "mov     x20 ,  0x3c0" );
        asm volatile( "msr     spsr_el1, x20" );                // State
        asm volatile( "msr     sp_el0,  %0" :: "r" (sp) );      // Stack pointer
        asm volatile( "eret    ");

        uart_puts("[Error]: cpio_exec unexpected close !\r\n"); // Never come there
        file_found += 1;
    }
}

void cpio_ls() {
    cpio_parse(cpio_ls_callback);
}

void cpio_cat() {
    // Get filename
    uart_puts("Filename: ");
    uart_gets(target_name);

    file_found = 0;

    cpio_parse(cpio_cat_callback);
    
    if(file_found == 0){
        uart_puts("Sorry, the specified file \"");
        uart_puts(target_name);
        uart_puts("\" could not be found.\r\n");
    }
}

void cpio_exec() {
    // Get filename
    uart_puts("Executable Filename: ");
    uart_gets(target_name);

    file_found = 0;

    cpio_parse(cpio_exec_callback);
    
    if(file_found == 0){
        uart_puts("Sorry, the specified file \"");
        uart_puts(target_name);
        uart_puts("\" could not be found.\r\n");
    }
}

void initramfs_callback(char *nodename, char *propname, struct fdt_prop* prop) {
    if (strncmp(nodename, "chosen", 7) == 0 && 
        strncmp(propname, "linux,initrd-start", 19) == 0) {
        DEVICETREE_CPIO_BASE = (void*)((long unsigned int) htonl(*((unsigned int*)(prop + 1))));
        uart_puts("Get DEVICETREE_CPIO_BASE ! \r\n");
    }
}