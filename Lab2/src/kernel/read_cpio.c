#include "stdlib.h"
#include "mini_uart.h"

typedef struct cpio_newc_header
{
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
} __attribute__((packed)) cpio_t;

void read_cpio(char *cpioDest)
{
    uart_send_string("Type     Offset   Size     Access rights\tFilename\n");

    while (!memcmp(cpioDest, "070701", 6) && memcmp(cpioDest + sizeof(cpio_t), "TRAILER!!!", 10))
    {
        cpio_t *header = (cpio_t *)cpioDest;
        int ns = hex2int(header->c_namesize, 8);
        int fs = hex2int(header->c_filesize, 8);
        // print out meta information
        uart_hex(hex2int(header->c_mode, 8)); // mode (access rights + type)
        uart_send(' ');
        uart_hex((unsigned int)((unsigned long)cpioDest) + sizeof(cpio_t) + ns);
        uart_send(' ');
        uart_hex(fs); // file size in hex
        uart_send(' ');
        uart_hex(hex2int(header->c_uid, 8)); // user id in hex
        uart_send('.');
        uart_hex(hex2int(header->c_gid, 8)); // group id in hex
        uart_send('\t');
        uart_send_string(cpioDest + sizeof(cpio_t)); // filename
        uart_send_string("\n");
        // jump to the next file
        if ((sizeof(cpio_t) + ns) % 4 != 0)
            cpioDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4) + fs);
        else
            cpioDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4) + fs);
    }
}

void read_content(char *cpioDest, char *filename)
{
    int flag = 0;
    while (!memcmp(cpioDest, "070701", 6) && memcmp(cpioDest + sizeof(cpio_t), "TRAILER!!!", 10))
    {
        cpio_t *header = (cpio_t *)cpioDest;
        int ns = hex2int(header->c_namesize, 8);
        // Check filename
        if (!memcmp(cpioDest + sizeof(cpio_t), filename, ns - 1))
        {
            flag = 1;
            break;
        }
        int fs = hex2int(header->c_filesize, 8);
        // jump to the next file
        if ((sizeof(cpio_t) + ns) % 4 != 0)
            cpioDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4) + fs);
        else
            cpioDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4) + fs);
    }
    // No hit
    if (flag == 0)
        printf("cat: %s: No such file\n", filename);
    // Found target file
    cpio_t *header = (cpio_t *)cpioDest;
    int ns = hex2int(header->c_namesize, 8);
    int fs = hex2int(header->c_filesize, 8);
    if ((sizeof(cpio_t) + ns) % 4 != 0)
        cpioDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4));
    else
        cpioDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4));

    // print content
    uart_send_string_of_size((char *)cpioDest, fs);
}