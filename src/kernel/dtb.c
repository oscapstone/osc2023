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

// const void (*initramfs_callback)(void *) = &callback;

void fdt_param_init(void *ptr) {
    base_addr = ptr;
    for(int i = 0; i < sizeof(struct fdt_header); i += 4) {
        *(uint32_t*)(((char*)&header + i)) = ntohi(*(uint32_t*)(((char*)ptr + i)));
    }
}

char *stack[128];
int sz = 0;

void push(char *ptr) {
    // this is bad since when sz > 128 it will crash
    stack[sz ++] = ptr;
}
char *top() {
    return stack[sz - 1];
}

void pop() {
    if(sz < 0) {
        uart_send_string("error occur\r\n");
    }
    sz -= 1;
}


void fdt_step(void *ptr, initramfs_callback_func f) {
    uint32_t curval = -1;
    uint32_t offset = 0;
    // char *name = simple_malloc(128);
    char data[1024];
    struct fdt_prop prop;
    char *name = simple_malloc(128);
    while(curval != FDT_END) {
        // uart_send_u64(ptr);
        curval = ntohi(*(uint32_t*)ptr);
        offset = 4;

        switch(curval) {
            case FDT_BEGIN_NODE:
                int i = 0;
                for(; *(char*)(ptr + i + offset) != '\0'; i ++) {
                    name[i] = *(char*)(ptr + i + offset);
                }
                name[i] = '\0';
                // if(name[0] == 'c') {
                //     uart_send_string(name);
                //     uart_send_string("\r\n");
                // }
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