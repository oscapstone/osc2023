#include "dtb/dtb.h"
#include "type.h"
#include "utils.h"
#include "peripherals/mini_uart.h"
#include "mem.h"


#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

struct fdt_header header;
char *base_addr;


void fdt_param_init(void *ptr) {
    base_addr = ptr;
    for(int i = 0; i < sizeof(struct fdt_header); i += 4) {
        *(uint32_t*)(((char*)&header + i)) = ntohi(*(uint32_t*)(((char*)ptr + i)));
    }
}



void fdt_step(void *ptr, initramfs_callback_func f) {
    uint32_t curval = -1;
    uint32_t offset = 0;
    // char data[1024];
    struct fdt_prop prop;
    char *name = simple_malloc(128);
    while(curval != FDT_END) {
        curval = ntohi(*(uint32_t*)ptr);
        offset = 4;

        switch(curval) {
            case FDT_BEGIN_NODE:
                int i = 0;
                for(; *(char*)(ptr + i + offset) != '\0'; i ++) {
                    name[i] = *(char*)(ptr + i + offset);
                }
                name[i] = '\0';

                offset += i + 4 - (i & 0x3);
                break;
            case FDT_PROP:
                prop.len = ntohi(*(uint32_t*)(ptr + offset));
                offset += 4;
                prop.nameoff = ntohi(*(uint32_t*)(ptr + offset));
                offset += 4;
                f(name, base_addr + header.off_dt_strings + prop.nameoff, (ptr + offset));
                offset += prop.len;
                offset += ((((prop.len & 0x3) ^ 0x3) + 1) & 0x3);
                break;
            case FDT_END_NODE:
            case FDT_NOP:
            case FDT_END:
                break;
            default:
        }
        ptr = ptr + offset;
    }
}

void fdt_traverse(initramfs_callback_func f) {
    char *struct_base = base_addr + header.off_dt_struct;
    fdt_step(struct_base, f);
}