#include "dtb.h"
#include "string.h"
#include "uart.h"

typedef void (*dtb_callback)(unsigned int node_type, char *name, void *value, unsigned int name_size);

extern char *cpio_start;
extern char *cpio_end;
char *dtb_base;

unsigned int endian_big2little(unsigned int x) {
    return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
}


void fdt_traverse(dtb_callback callback)
{
    struct fdt_header *header = (struct fdt_header *)dtb_base;
    

    if (endian_big2little(header->magic) != 0xD00DFEED)
    {
        uart_printf("fdt_traverse: wrong magic in fdt_traverse\n");
        uart_printf("expect: 0XD00DFEED, get: %x\n", endian_big2little(header->magic));
        return;
    }

    unsigned int struct_size = endian_big2little(header->size_dt_struct);
    char *dt_struct_ptr = (char *)((char *)header + endian_big2little(header->off_dt_struct));
    char *dt_strings_ptr = (char *)((char *)header + endian_big2little(header->off_dt_strings));

    char *end = (char *)dt_struct_ptr + struct_size;
    char *pointer = dt_struct_ptr;

    while (pointer < end)
    {
        unsigned int token_type = endian_big2little(*(unsigned int *)pointer);
        pointer += 4;

        switch (token_type)
        {
        case FDT_BEGIN_NODE:
            pointer += strlen(pointer);
            pointer += (4 - (unsigned long long)pointer % 4);
            break;
        case FDT_END_NODE:
            break;
        case FDT_PROP:
            unsigned int len = endian_big2little(*(unsigned int *)pointer);
            pointer += 4;
            char *name = (char *)dt_strings_ptr + endian_big2little(*(unsigned int *)pointer);
            pointer += 4;
            callback(token_type, name, pointer, len);
            pointer += len;
            if ((unsigned long long)pointer % 4 != 0)
                pointer += 4 - (unsigned long long)pointer % 4; // alignment 4 byte
            break;
        case FDT_NOP:
            break;
        case FDT_END:
            break;
        default:
            return;
        }
    }
}

void initramfs_callback(unsigned int node_type, char *name, void *value, unsigned int name_size)
{
    if (!strcmp(name, "linux,initrd-start"))
        cpio_start = (char *)(unsigned long long)endian_big2little(*(unsigned int *)value);

    if (!strcmp(name, "linux,initrd-end"))
        cpio_end = (char *)(unsigned long long)endian_big2little(*(unsigned int *)value);
}