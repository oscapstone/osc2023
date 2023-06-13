#include <dt17.h>
#include <type.h>
#include <mini_uart.h>
#include <string.h>

uint32 initramfs_check_cnt = 0;

static void print_tab(int level){
    while(level--)
        uart_printf("\t");
}

static void dump(char *start, int len)
{
    while (len--) {
        char c = *start++;
        if ((0x20 <= c && c <= 0x7e)) {
            uart_printf("%c", c);
        } else {
            uart_printf("%x", c);
        }
    }
}

static void parse_dt_struct(char *dt_struct, char *dt_strings, fdt_parser parser){
    char *cur = dt_struct;
    int level = 0;

    while(1){
        cur = (char*)ALIGN((fdt64_t)cur, 4);
        struct fdt_property *prop;

        fdt32_t tag = fdt32_ld((fdt32_t*)cur);
        switch(tag){
            
            case FDT_BEGIN_NODE:
                if(parser(level, cur, dt_strings))
                    return;
                level++;
                cur += sizeof(fdt32_t);
                cur += strlen(cur) + 1;
                while(*cur!=0)
                    cur++;
                break;
            case FDT_END_NODE:
                level--;
                if(parser(level, cur, dt_strings))
                    return;
                cur += sizeof(fdt32_t);
                break;
            case FDT_PROP:
                prop = (struct fdt_property *)(cur+sizeof(fdt32_t));
                if(parser(level, cur, dt_strings))
                    return;
                cur += sizeof(fdt32_t);
                cur += sizeof(struct fdt_property);
                cur += fdtp_len(prop);
                break;
            case FDT_NOP:
                if(parser(level, cur, dt_strings))
                    return;
                cur += sizeof(fdt32_t);
                break;
            case FDT_END:
                parser(level, cur, dt_strings);
                return;  
        }
    }
}

void parse_dtb(char *fdt, fdt_parser parser){

    struct fdt_header *fhdr = (struct fdt_header *)fdt;
    if(fdt_magic(fhdr)!=FDT_MAGIC_NUM)
        uart_printf("[x] Invalid fdt_header\r\n");
    
    if(fdt_last_comp_version(fhdr) > 17)
        uart_printf("[x] Only support version <= 17 fdt");
    
    char *dt_struct = fdt + fdt_off_dt_struct(fhdr);
    char *dt_strings = fdt + fdt_off_dt_strings(fhdr);

    parse_dt_struct(dt_struct, dt_strings, parser);
}

int initramfs_parse_fdt(int level, char *cur, char *dt_strings){
    fdt32_t tag = fdt32_ld((fdt32_t*)cur);
    cur += sizeof(fdt32_t);

    if(tag == FDT_PROP){
        struct fdt_property *prop =(struct fdt_property *) cur;
        cur += sizeof(struct fdt_property);
        if(!strcmp("linux,initrd-start", dt_strings + fdtp_nameoff(prop))){
            _initramfs_addr = fdt32_ld((fdt32_t*)cur);
            uart_printf("[*] initrd addr: %x\r\n", _initramfs_addr);
            initramfs_check_cnt++;
        }
        else if(!strcmp("linux,initrd-end", dt_strings + fdtp_nameoff(prop))){
            _initramfs_end = fdt32_ld((fdt32_t*)cur);
            uart_printf("[*] initrd end: %x\r\n", _initramfs_end);
            initramfs_check_cnt++;
        }
        if(initramfs_check_cnt==2)
            return 1;
    }
    return 0;
}

void initramfs_init(char *fdt_base){
    _initramfs_addr = 0;
    parse_dtb(fdt_base, initramfs_parse_fdt);
    if(!_initramfs_addr)
        uart_printf("[x] Cannot find initrd address in dtb!\r\n");
}

static int dtb_traverse_parser(int level, char *cur, char *dt_strings){
    fdt32_t tag = fdt32_ld((fdt32_t*)cur);

    switch(tag){
        case FDT_BEGIN_NODE:
            print_tab(level);
            cur += sizeof(fdt32_t);
            uart_printf("[*] Node: %s\r\n", cur);
            break;
        case FDT_END_NODE:
            print_tab(level);
            uart_printf("[*] Node end\r\n");
            break;
        case FDT_PROP:
            print_tab(level);
            cur += sizeof(fdt32_t);
            struct fdt_property *prop = (struct fdt_property*)cur;
            cur += sizeof(struct fdt_property);
            uart_printf("[*] %s:", dt_strings + fdtp_nameoff(prop));
            dump(cur, fdtp_len(prop));
            uart_printf("\r\n");
            break;
        case FDT_NOP:
            break;
        case FDT_END:
            uart_printf("[*] End of device tree\r\n");
    }
    return 0;
}

void dtb_traverse(char *fdt_base){
    parse_dtb(fdt_base, dtb_traverse_parser);
}