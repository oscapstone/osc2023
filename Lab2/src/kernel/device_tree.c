#include <stdint.h>
#include "peripherals/device_tree.h"
#include "stdlib.h"

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

static uint64_t pad_to_4(void *num);
static uint32_t rev32(uint32_t val);
static uint64_t endian_rev(void *input, int dtype_size);

int get_property(void *_dtb)
{
    uintptr_t dtb = (uintptr_t)_dtb;
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
            // printf("prop_name = %s\n", prop_name);

            if (!strcmp(FDT_CPIO_INITRAMFS_PROPNAME, prop_name))
            {
                printf("FOUND\n");
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

void initramfs_callback()
{
}

void fdt_traverse(void *dtb)
{
    printf("dtb = %d\n", dtb);
    int prop;
    prop = get_property((void *)dtb);
    printf("%d\n", prop);

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