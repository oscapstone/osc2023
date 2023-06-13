#include "_cpio.h"
#include "utils_c.h"
#include "mini_uart.h"

unsigned long hex2dec(char *s) // hex to decimal int
{
    unsigned long r = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (s[i] >= '0' && s[i] <= '9')
        {
            r = r * 16 + (s[i] - '0');
        }
        else if (s[i] >= 'a' && s[i] <= 'f')
        {
            r = r * 16 + (s[i] - 'a' + 10);
        }
        else if (s[i] >= 'A' && s[i] <= 'F')
        {
            r = r * 16 + (s[i] - 'A' + 10);
        }
    }
    return r;
}

char *findFile(char *name)
{
    char *addr = cpio_addr;
    while (utils_str_compare((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0)
    {
        if ((utils_str_compare((char *)(addr + sizeof(cpio_header)), name) == 0))
        {
            return addr;
        }
        cpio_header *header = (cpio_header *)addr;
        unsigned long pathname_size = hex2dec(header->c_namesize);
        unsigned long file_size = hex2dec(header->c_filesize);

        unsigned long headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);
        addr += (headerPathname_size + file_size);
    }
    return 0;
}
void cpio_ls()
{
    char *addr = cpio_addr;
    // while (utils_str_compare((char *)(addr + sizeof(cpio_header) / sizeof(char)), "TRAILER!!!") != 0)
    while (utils_str_compare((char *)(addr + sizeof(cpio_header)), "TRAILER!!!") != 0)
    {
        cpio_header *header = (cpio_header *)addr;
        unsigned long pathname_size = hex2dec(header->c_namesize);
        unsigned long file_size = hex2dec(header->c_filesize); // 因為filename後必跟一個nul,所以call出來的長度多一
        unsigned long headerPathname_size = sizeof(cpio_header) + pathname_size;
        // unsigned long headerPathname_size = sizeof(cpio_header) / sizeof(char) + pathname_size;

        // 將目前的size往4對齊補足,無條件進位
        align(&headerPathname_size, 4);
        align(&file_size, 4);

        uart_send_string(addr + sizeof(cpio_header)); // print the file name
        // uart_send_string(addr + sizeof(cpio_header) / sizeof(char)); // print the file name
        uart_send_string("\n");

        addr += (headerPathname_size + file_size);
        // printf("*addr is : %c\n", *addr);
        // uart_hex(hex2dec(addr));
    }
}

void cpio_cat(char *filename)
{
    char *target = findFile(filename);
    if (target)
    {
        cpio_header *header = (cpio_header *)target;
        unsigned long pathname_size = hex2dec(header->c_namesize);
        unsigned long file_size = hex2dec(header->c_filesize);
        unsigned long headerPathname_size = sizeof(cpio_header) + pathname_size;

        align(&headerPathname_size, 4);
        align(&file_size, 4);

        char *file_content = target + headerPathname_size; // file_content's addr
        for (unsigned int i = 0; i < file_size; i++)
        {
            uart_send(file_content[i]); // print the file content
        }
        uart_send_string("\n");
    }
    else
    {
        uart_send_string("Not found the file\n");
    }
}
