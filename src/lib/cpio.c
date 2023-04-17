#include <cpio.h>
#include <mini_uart.h>
#include <string.h>
#include <mem.h>
#include <mm.h>

static uint32 cpio_read_hex(char *p){
    uint32 result = 0;

    for(int i = 0 ; i < 8 ; i++){
        char c = *p;

        result <<= 4;
        if(c >= '0' && c <= '9')
            result += c-'0';
        else if('A' <= c && c <= 'F')
            result += c-'A'+10;

        p++;
    }

    return result;
}

void cpio_ls(char *cpio){
    char *cur = cpio;

    while(1){
        struct cpio_newc_header *pheader = (struct cpio_newc_header *)cur;
        cur += sizeof(struct cpio_newc_header);
        if(!strcmp(pheader->c_magic, "070701")){
            uart_printf("Only support new ASCII format for cpio. \r\n");
            return;
        }
        
        uint32 namesize = cpio_read_hex(pheader->c_namesize);
        uint32 filesize = cpio_read_hex(pheader->c_filesize);

        // The pathname is followed by NUL bytes so that the total size of the 
        // fixed header plus pathname is a multiple of four. Likewise, the file
        // data is padded to a multiple of four bytes
        char *filename = cur;
        uint32 aligned_namesize = ALIGN(sizeof(struct cpio_newc_header) + namesize, 4) - sizeof(struct cpio_newc_header);
        uint32 aligned_filesize = ALIGN(filesize, 4);

        cur += aligned_namesize;
        cur += aligned_filesize;

        if(!strcmp(filename, "TRAILER!!!"))
            return;
        
        uart_printf("%s\r\n", filename);
    }
}

void cpio_cat(char *cpio, char *filename){
    char *cur = cpio;

    while(1){
        struct cpio_newc_header *pheader = (struct cpio_newc_header *) cur;
        cur += sizeof(struct cpio_newc_header);
        if(!strcmp(pheader->c_magic, "070701")){
            uart_printf("Only support new ASCII format for cpio. \r\n");
            return;
        }
        
        uint32 namesize = cpio_read_hex(pheader->c_namesize);
        uint32 filesize = cpio_read_hex(pheader->c_filesize);

        // The pathname is followed by NUL bytes so that the total size of the 
        // fixed header plus pathname is a multiple of four. Likewise, the file
        // data is padded to a multiple of four bytes
        uint32 aligned_namesize = ALIGN(sizeof(struct cpio_newc_header) + namesize, 4) - sizeof(struct cpio_newc_header);
        uint32 aligned_filesize = ALIGN(filesize, 4);

        char *curfilename = cur;
        cur += aligned_namesize;
        char *curfilecontent = cur;
        cur += aligned_filesize;

        if(!strcmp(curfilename, filename)){
            uart_sendn(curfilecontent, aligned_filesize);
            uart_printf("\r\n");
            return;
        }

        if(!strcmp(curfilename, "TRAILER!!!")){
            uart_printf("%s: no such file.\r\n", filename);
            return;
        }
    }
}

char *cpio_load_prog(char *cpio, char *filename){
    char *cur = cpio;

    while(1){
        struct cpio_newc_header *pheader = (struct cpio_newc_header *) cur;
        cur += sizeof(struct cpio_newc_header);
        if(!strcmp(pheader->c_magic, "070701")){
            uart_printf("Only support new ASCII format for cpio. \r\n");
            return NULL;
        }
        
        uint32 namesize = cpio_read_hex(pheader->c_namesize);
        uint32 filesize = cpio_read_hex(pheader->c_filesize);

        // The pathname is followed by NUL bytes so that the total size of the 
        // fixed header plus pathname is a multiple of four. Likewise, the file
        // data is padded to a multiple of four bytes
        uint32 aligned_namesize = ALIGN(sizeof(struct cpio_newc_header) + namesize, 4) - sizeof(struct cpio_newc_header);
        uint32 aligned_filesize = ALIGN(filesize, 4);

        char *curfilename = cur;
        cur += aligned_namesize;
        char *curfilecontent = cur;
        cur += aligned_filesize;

        if(!strcmp(curfilename, filename)){
            char *mem = (char*)kmalloc(filesize);
            memncpy(mem, curfilecontent, (unsigned long)filesize);
            return mem;
        }

        if(!strcmp(curfilename, "TRAILER!!!")){
            uart_printf("%s: no such file.\r\n", filename);
            return NULL;
        }
    }
}