#include <ramdisk.h>

#include <string.h>

#include <utils.h>
#include <uart.h>
#include <allocator.h>
#include <devicetree.h>

int isinit;
void *_randisk_begin;

typedef struct{
    int found;
    char *initrd_start;
} ramdisk_data;

int ramdisk_fdt_callback(char* node, const char *name, void *data){
    if(strcmp(name, "chosen")) return 0;
    ramdisk_data* _data = (ramdisk_data*) data;
    fdt_prop *prop;
    while(prop = fdt_nextprop(node + 1, &node)){
        if(!strcmp(prop->name,"linux,initrd-start")){
            _data->initrd_start = (char *)(uint64_t)ntohl(*(uint32_t*)prop->value);
        }
    }
    _data->found = 1;
    return 1;
}

int ramdisk_get_addr(){
    ramdisk_data data;
    data.initrd_start = 0;
    data.found = 0;
    fdt_traverse(ramdisk_fdt_callback, (void *)&data);
    if(data.found){
        uart_print("/chosen linux,initrd-start: ");
        uart_print_hex((uint64_t)data.initrd_start, 64);
        newline();
        _randisk_begin = data.initrd_start;
        return 0;
    }
    return -1;
}

cpio_file* cpio_parse(){
    if(!isinit){
        int initres = ramdisk_get_addr();
        if(!initres) isinit = 1;
        else return 0;
    }
    char *ptr = (char *) _randisk_begin;
    char *name = ptr + sizeof(struct cpio_newc_header);
    struct cpio_newc_header* cur_cpio_header;
    cpio_file *head = NULL;
    cpio_file *prev = NULL;
    cpio_file *cur = NULL;
    while(1){
        cur_cpio_header = (struct cpio_newc_header*) ptr;
        name = ptr + sizeof(struct cpio_newc_header);
        if(strcmp("TRAILER!!!", name) == 0) break;
        cur = (cpio_file*) simple_malloc (sizeof(cpio_file));
        if(head == NULL) head = cur;
        cur->header = ptr;
        cur->namesize = hex2u32_8(cur_cpio_header->c_namesize);
        cur->filesize = hex2u32_8(cur_cpio_header->c_filesize);
        cur->name = name;
        cur->content = (char*) align((uint64_t) name + cur->namesize - 1);
        cur->next = 0;
        ptr = (char *) align((uint64_t) cur->content + cur->filesize - 1);
        if(prev != NULL) prev->next = cur;
        prev = cur;
    }
    return head;
}