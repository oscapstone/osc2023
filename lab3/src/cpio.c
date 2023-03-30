#include "mini_uart.h"
#include "utils.h"
#include "dtb.h"
#include <stdint.h>

static void *mem = 0x8000000;

typedef struct {
    char	   c_magic[6];
    char	   c_ino[8];
    char	   c_mode[8];
    char	   c_uid[8];
    char	   c_gid[8];
    char	   c_nlink[8];
    char	   c_mtime[8];
    char	   c_filesize[8];
    char	   c_devmajor[8];
    char	   c_devminor[8];
    char	   c_rdevmajor[8];
    char	   c_rdevminor[8];
    char	   c_namesize[8];
    char	   c_check[8];
} cpio_t;

/**
 * List the contents of an archive
 */
void initrd_list()
{
    char *buf = (char *)mem;
    uart_puts("Type     Position   Size     NameLen\tFilename\n");
    // if it's a cpio archive. Cpio also has a trailer entry
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_t),"TRAILER!!",9)) {
        cpio_t *header = (cpio_t*)buf;
        int ns=hex2bin(header->c_namesize,8);
        int fs=hex2bin(header->c_filesize,8);
        // print out meta information
        uart_hex(hex2bin(header->c_mode,8));  // mode (access rights + type)
        uart_send(' ');
        uart_hex((unsigned int)((unsigned long)buf)+sizeof(cpio_t)+ns);
        uart_send(' ');
        uart_hex(fs);                       // file size in hex
        uart_send(' ');
        uart_hex(ns);   // namesize for debugging
        uart_send('\t');
        uart_puts(buf+sizeof(cpio_t));      // filename
        uart_puts("\n");
        // jump to the next file
        int off = ((ns+fs+sizeof(cpio_t))%4);
        if (off!=0) 
            off = 4-off;
        buf+=(sizeof(cpio_t)+ns+fs+off);
    }
}

void cat_list () {
    char *buf = (char *)mem;
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_t),"TRAILER!!",9)) {
        // uart_puts("\n");
        cpio_t *header = (cpio_t*)buf;
        int ns=hex2bin(header->c_namesize,8);
        int fs=hex2bin(header->c_filesize,8);
        uart_puts(buf+sizeof(cpio_t));      // filename
        uart_puts("\n");
        if (fs)
            uart_puts(buf+sizeof(cpio_t)+ns);
        uart_puts("\n");
        // jump to the next file
        int off = ((ns+fs+sizeof(cpio_t))%4);
        if (off!=0) 
            off = 4-off;
        buf+=(sizeof(cpio_t)+ns+fs+off);
    }
}

unsigned int * load_prog (void * prog) {
    char *buf = (char *)mem;
    char *load;
    unsigned int ret = 0;
    
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_t),"TRAILER!!",9)) {
        cpio_t *header = (cpio_t*)buf;
        int ns=hex2bin(header->c_namesize,8);
        int fs=hex2bin(header->c_filesize,8);
        if(!memcmp(buf+sizeof(cpio_t), (char *)prog, ns-1)){
            uart_puts("HIT\n");
            // Only one user program, hard writing return address to 0x90000000 for further more applications
            ret = buf+sizeof(cpio_t)+ns;
            return ret + (8-(ret%8));
        }
        // jump to the next file
        int off = ((ns+fs+sizeof(cpio_t))%4);
        if (off!=0) 
            off = 4-off;
        buf+=(sizeof(cpio_t)+ns+fs+off);
    }
    return 0;
}

int callback_initramfs(void * addr){
    uint32_t t = *((uint32_t*)addr);
    uart_hex(t);
    mem = (void*)(bswap_32(t));
    return;
}

int get_initramfs(){
    return mem;
}

void print_initramfs(){
    uart_hex(mem);
}