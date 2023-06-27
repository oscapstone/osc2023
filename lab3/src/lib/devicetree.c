#include <devicetree.h>

#include <string.h>
#include <allocator.h>
#include <utils.h>
#include <uart.h>

void *_devicetree_begin = (void *) 0xffffffff;

void fdt_traverse(initramfs_callback f, void *data){
    char *fdt = (char *) _devicetree_begin;
    if((uint64_t)fdt == 0xffffffff) return;

    fdt_header *header = (fdt_header *) fdt;

    char *fdt_struct_begin = fdt + ntohl(header->off_dt_struct);
    char *fdt_string_begin = fdt + ntohl(header->off_dt_strings);
    char *fdt_struct_end = fdt_struct_begin + ntohl(header->size_dt_struct);

    int isend = 0;
    char *node_name;
    char *prop_name;
    for(char *node = fdt_nextnode(fdt_struct_begin-1, fdt_struct_end); node && !isend; node = fdt_nextnode(node, fdt_struct_end)){
        f(node, node + 4, data);
    }
}

char *fdt_nextnode(char *fdt_addr, char *fdt_struct_end){
    fdt_addr = (char *) align((uint64_t) fdt_addr);
    int isend = 0;
    while((uint64_t)fdt_addr < (uint64_t)fdt_struct_end && !isend) {
        switch (ntohl(*(uint32_t*)fdt_addr)){
            case FDT_BEGIN_NODE: {
                return fdt_addr;
            }case FDT_PROP: {
                fdt_addr += (uint64_t)ntohl(*(uint32_t*)(fdt_addr + 4)) + 8;
                break;
            }case FDT_END: {
                isend = 1;
                break;
            }case FDT_END_NODE: {
            }case FDT_NOP: {
                break;
            }
        }
        fdt_addr = (char *) align((uint64_t) fdt_addr);
    }
    return 0;
}

fdt_prop *fdt_nextprop(char *fdt_addr, char **nexttok){
    char *fdt = (char *)_devicetree_begin;
    if((uint64_t)fdt == 0xffffffff) return 0;

    fdt_header *header = (fdt_header *) fdt;

    char *fdt_string_begin = fdt + ntohl(header->off_dt_strings);

    fdt_addr = (char *) align((uint64_t) fdt_addr - 1);
    int isend = 0;
    fdt_prop *prop;
    while(!isend) {
        switch (ntohl(*(uint32_t*)fdt_addr)){
            case FDT_PROP:
                prop = (fdt_prop*) simple_malloc(sizeof(fdt_prop));
                if(prop == NULL){
                    uart_print("Error: fdt_nuxtprop");
                    newline();
                    return NULL;
                }
                prop->len = ntohl(*(uint32_t*)(fdt_addr+4));
                prop->name = fdt_string_begin + ntohl(*(uint32_t *) (fdt_addr + 8));
                prop->value = fdt_addr + 12;
                *nexttok = (void *) fdt_addr + 8 + ntohl(*(uint32_t*) (fdt_addr + 4));
                return prop;
            case FDT_END:
            case FDT_BEGIN_NODE:
            case FDT_END_NODE:
                isend = 1;
                break;
            case FDT_NOP:
                break;
        }
        fdt_addr = (char *) align((uint64_t) fdt_addr);
    }
    return 0;
}

