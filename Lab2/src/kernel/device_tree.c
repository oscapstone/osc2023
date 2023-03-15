#include <stdint.h>
#include "peripherals/device_tree.h"
#include "stdlib.h"
#include "mini_uart.h"
#include "device_tree.h"

typedef struct fdt_header
{
    uint32_t magic; // big-endian
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} fdt_header;

typedef struct
{
    uint32_t len;
    uint32_t nameoff;
} fdt_prop;

char *cpioDest;

static uint64_t pad_to_4(void *num);
static uint32_t rev32(uint32_t val);
static uint64_t endian_rev(void *input, int dtype_size);

int initramfs_callback(void *dtb)
{
    fdt_header *header = (fdt_header *)dtb;

    // Magic Number
    if (rev32(header->magic) != FDT_MAGIC)
    {
        printf("Wrong Magic Number 0x%08x\n", rev32(header->magic));
        return -1;
    }

    uint8_t *off_dt_struct = (uint8_t *)header + rev32(header->off_dt_struct);
    uint8_t *off_dt_strings = (uint8_t *)header + rev32(header->off_dt_strings);

    uint8_t *p = off_dt_struct;
    fdt_prop *prop = NULL;
    char *prop_name = NULL;
    char *prop_val = NULL;

    // Traverse the device tree structure
    while (1)
    {
        const uint32_t token = rev32(*(uint32_t *)p);
        switch (token)
        {
        case FDT_BEGIN_NODE:
            // Move to the next entry
            p += 4;
            p += strlen((char *)(p)) + 1; // +1 for null terminated string
            p += pad_to_4(p);
            break;
        case FDT_END_NODE:
            // Move to the next entry
            p += 4;
            break;
        case FDT_PROP:
            p += 4;
            prop = (fdt_prop *)p;
            p += sizeof(fdt_prop);
            prop_name = (char *)off_dt_strings + rev32(prop->nameoff);
            prop_val = (char *)p;
            if (!strcmp(FDT_CPIO_INITRAMFS_PROPNAME, prop_name))
            {
                uint64_t addr = (uint64_t)rev32(*((uint32_t *)prop_val));
                printf("initramfs_addr at %p\n", addr);
                cpioDest = (char *)addr;
            }
            p += rev32(prop->len);
            p += pad_to_4(p);
            break;
        case FDT_NOP:
            p += 4;
            break;
        case FDT_END:
            return rev32(header->totalsize);
        default:
            // Invalid entry
            printf("Invalid FDT entry\n");
            return -1;
        }
    }

    // Property not found
    return -1;
}

int dtb_parser(void *dtb)
{
    fdt_header *header = (fdt_header *)dtb;

    // Magic Number
    if (rev32(header->magic) != FDT_MAGIC)
    {
        printf("Wrong Magic Number 0x%08x\n", rev32(header->magic));
        return -1;
    }

    uint8_t *off_dt_struct = (uint8_t *)header + rev32(header->off_dt_struct);
    uint8_t *off_dt_strings = (uint8_t *)header + rev32(header->off_dt_strings);

    int depth = 0;
    uint8_t *p = off_dt_struct;
    char *node_name = NULL;
    fdt_prop *prop = NULL;
    char *prop_name = NULL;
    char *prop_val = NULL;

    // Traverse the device tree structure
    while (1)
    {
        const uint32_t token = rev32(*(uint32_t *)p);
        switch (token)
        {
        case FDT_BEGIN_NODE:
            // Move to the next entry
            p += 4;
            node_name = (char *)p;
            if (depth == 0)
                printf("\\ {\n");
            else
            {
                uart_send_space(depth * 3);
                printf("%s {\n", node_name);
            }
            p += strlen((char *)(p)) + 1; // +1 for null terminated string
            p += pad_to_4(p);
            depth++;
            break;
        case FDT_END_NODE:
            // Move to the next entry
            p += 4;
            depth--;
            uart_send_space(depth * 3);
            printf("};\n");
            printf("\n");
            break;
        case FDT_PROP:
            p += 4;
            prop = (fdt_prop *)p;
            p += sizeof(fdt_prop);
            prop_name = (char *)off_dt_strings + rev32(prop->nameoff);
            prop_val = (char *)p;

            if (!strcmp(prop_name, "#address-cells") || !strcmp(prop_name, "#size-cells") || !strcmp(prop_name, "interrupt-parent"))
            {
                // <u32>
                uart_send_space(depth * 3);
                printf("%s = <%d>;\n", prop_name, rev32(*((uint32_t *)prop_val)));
            }
            else if (!strcmp(prop_name, "model") || !strcmp(prop_name, "status") || !strcmp(prop_name, "name") || !strcmp(prop_name, "device_type") ||
                     !strcmp(prop_name, "chassis-type") || !strcmp(prop_name, "bootargs") || !strcmp(prop_name, "stdout-path") || !strcmp(prop_name, "stdin-path") ||
                     !strcmp(prop_name, "power-isa-version") || !strcmp(prop_name, "mmu-type") || !strcmp(prop_name, "label") || !strcmp(prop_name, "phy-connection-type"))
            {
                // <string>
                uart_send_space(depth * 3);
                printf("%s = \"%s\";\n", prop_name, prop_val);
            }
            else if (!strcmp(prop_name, "compatible"))
            {
                // <stringlist>
                uart_send_space(depth * 3);
                printf("%s = \"%s\";\n", prop_name, prop_val);
            }
            else
            {
                uart_send_space(depth * 3);
                printf("%s = %s;\n", prop_name, prop_val);
            }

            p += rev32(prop->len);
            p += pad_to_4(p);
            break;
        case FDT_NOP:
            p += 4;
            break;
        case FDT_END:
            return rev32(header->totalsize);
        default:
            // Invalid entry
            printf("Invalid FDT entry\n");
            return -1;
        }
    }

    // Property not found
    return -1;
}

void fdt_traverse(fdt_callback cb, char *dtb)
{
    if (cb(dtb) == -1)
        printf("fdt_traverse failed.\n");

    return;
}

static uint64_t pad_to_4(void *num)
{
    uint64_t modded = ((uint64_t)num) % 4;
    return modded == 0 ? 0 : 4 - modded;
}

static uint32_t rev32(uint32_t val)
{
    return (uint32_t)endian_rev(&val, 4);
}

/** Transform data from litle to big endian, or from big to little endian
 * @param input: Pointer to a value that needs to be transformed
 * @param dtype_size: data type size, size of each item in bytes. Possible value: 1, 2, 4, 8
 */
static uint64_t endian_rev(void *input, int dtype_size)
{
    const uint8_t *ptr = (uint8_t *)input;
    uint64_t ret = 0;

    switch (dtype_size)
    {
    // int8_t, uint8_t
    case 1:
        // No need to transform to big endian since the data type size is 1 byte
        break;

    // int16_t, uint16_t
    case 2:
        ret = (ptr[0] << 8) | ptr[1];
        break;

    // int32_t, uint32_t
    case 4:
        ret = (ptr[0] << 24) |
              (ptr[1] << 16) |
              (ptr[2] << 8) |
              ptr[3];
        break;

        // int64_t, uint64_t
        // case 8:
        //     ret = (ptr[0] << 56) |
        //           (ptr[1] << 48) |
        //           (ptr[2] << 40) |
        //           (ptr[3] << 32) |
        //           (ptr[4] << 24) |
        //           (ptr[5] << 16) |
        //           (ptr[6] << 8) |
        //           ptr[7];
        //     break;

    default:
        printf("[Error] Endian transformation(%d) not implemented. @line %d, file:%s\r\n", dtype_size, __LINE__, __FILE__);
        break;
    }
    return ret;
}