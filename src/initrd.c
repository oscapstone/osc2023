/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "uart.h"
#include "shell.h"
#include "my_string.h"
#include "file.h"

// add memory compare, gcc has a built-in for that, clang needs implementation
#ifdef __clang__
int memcmp(void *s1, void *s2, int n)
{
    unsigned char *a=s1,*b=s2;
    while(n-->0){ if(*a!=*b) { return *a-*b; } a++; b++; }
    return 0;
}
#else
#define memcmp __builtin_memcmp
#endif

#define buf_size 128

typedef struct {
    char magic[6];
    char ino[8];
    char mode[8];
    char uid[8];
    char gid[8];
    char nlink[8];
    char mtime[8];
    char filesize[8];
    char devmajor[8];
    char devminor[8];
    char rdevmajor[8];
    char rdevminor[8];
    char namesize[8];
    char check[8];
} __attribute__((packed)) cpio_f;

/**
 * List the contents of an archive
 */
void initrd_list(char *buf)
{
    uart_puts("Type     Offset   Size     Access rights\tFilename\n");

        // if it's a cpio newc archive. Cpio also has a trailer entry
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_f),"TRAILER!!",9)) {
        cpio_f *header = (cpio_f*) buf;
        int ns = hex2bin(header->namesize, 8);
        int fs = hex2bin(header->filesize, 8);
        // print out meta information
        uart_hex(hex2bin(header->mode, 8));  // mode (access rights + type)
        uart_send(' ');
        uart_hex((unsigned int)((unsigned long)buf)+sizeof(cpio_f)+ns);
        uart_send(' ');
        uart_hex(fs);                       // file size in hex
        uart_send(' ');
        uart_hex(hex2bin(header->uid, 8));   // user id in hex
        uart_send('.');
        uart_hex(hex2bin(header->gid, 8));   // group id in hex
        uart_send('\t');
        uart_puts(buf+sizeof(cpio_f));      // filename
        uart_puts("\n");
        // jump to the next file
        buf+=(sizeof(cpio_f) + ns + fs);
    }
}

/**
 * List the filenames of an archive
 */
void initrd_ls(char *buf)
{
    uart_puts(".\n");

    // if it's a cpio newc archive. Cpio also has a trailer entry
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_f),"TRAILER!!",9)) {
        cpio_f *header = (cpio_f*) buf;
        int ns = hex2bin(header->namesize, 8);
        int fs = hex2bin(header->filesize, 8);
        // print out filename
        uart_puts(buf+sizeof(cpio_f));      // filename
        uart_puts("\n");
        // jump to the next file
        buf+=(sizeof(cpio_f) + ns + fs);
    }
}

void initrd_cat(char *buf)
{
    char buffer[buf_size];

    // get file name
    uart_puts("filename: ");
    shell_input(buffer);

    // if it's a cpio newc archive. Cpio also has a trailer entry
    while(!memcmp(buf,"070701",6) && memcmp(buf+sizeof(cpio_f),"TRAILER!!",9)) {
        cpio_f *header = (cpio_f*) buf;
        int ns = hex2bin(header->namesize, 8);
        int fs = hex2bin(header->filesize, 8);

        // check filename with buffer
        if (!strcmp(buffer, buf + sizeof(cpio_f))) {
            if (fs == 0) {
                uart_send('\n');
                uart_puts("This is a directory\n");
                return;
            } else {
                uart_send('\n');
                readfile(buf + sizeof(cpio_f) + ns, fs);
                return;
            }
        }

        // jump to the next file
        buf+=(sizeof(cpio_f) + ns + fs);
    }
    uart_puts("File not found\n");
}
