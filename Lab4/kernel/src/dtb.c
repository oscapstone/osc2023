#include "dtb.h"
#include "uart.h"
#include "cpio.h"
#include "type.h"
#include "utils.h"
#include "mm.h"

void* CPIO_DEFAULT_START;
void* CPIO_DEFAULT_END;

/* https://abcamus.github.io/2016/12/26/uboot设备树介绍/ */
/* Tree structure to describe the hardware device */

extern char* dtb_ptr;

uint32_t uint32_endian_big2lttle(uint32_t data){

    char* r = (char*)&data;
    return (r[3]<<0) | (r[2]<<8) | (r[1]<<16) | (r[0]<<24);

}


uint64_t uint64_endian_big2lttle(uint64_t data)
{
    char *r = (char *)&data;
    return ((unsigned long long)r[7] << 0) | ((unsigned long long)r[6] << 8) | ((unsigned long long)r[5] << 16) | ((unsigned long long)r[4] << 24) | ((unsigned long long)r[3] << 32) | ((unsigned long long)r[2] << 40) | ((unsigned long long)r[1] << 48) | ((unsigned long long)r[0] << 56);
}


/* Device tree parser */
void traverse_device_tree(void *dtb_ptr,dtb_callback callback)
{

    struct fdt_header* header = dtb_ptr;
    if(uint32_endian_big2lttle(header->magic) != 0xD00DFEED)
    {
        uart_puts("traverse_device_tree : wrong magic in traverse_device_tree");
        return;
    }




    uint32_t struct_size = uint32_endian_big2lttle(header->size_dt_struct);
    char* dt_struct_ptr = (char*)((char*)header + uint32_endian_big2lttle(header->off_dt_struct));
    char* dt_strings_ptr = (char*)((char*)header + uint32_endian_big2lttle(header->off_dt_strings));

    char* end = (char*)dt_struct_ptr + struct_size;
    char* pointer = dt_struct_ptr;

    

    while(pointer < end)
    {

        uint32_t token_type = uint32_endian_big2lttle(*(uint32_t*)pointer);

        pointer += 4;
        if(token_type == FDT_BEGIN_NODE)
        {
            callback(token_type,pointer,0,0);
            pointer += strlen(pointer);
            pointer += 4 - (unsigned long long)pointer%4;           //alignment 4 byte
        }else if(token_type == FDT_END_NODE)
        {
            callback(token_type,0,0,0);
        }else if(token_type == FDT_PROP)
        {
            uint32_t len = uint32_endian_big2lttle(*(uint32_t*)pointer);
            pointer += 4;
            char* name = (char*)dt_strings_ptr + uint32_endian_big2lttle(*(uint32_t*)pointer);
            pointer += 4;
            callback(token_type,name,pointer,len);
            pointer += len;
            if((unsigned long long)pointer % 4 !=0)pointer += 4 - (unsigned long long)pointer%4;   //alignment 4 byte
        }else if(token_type == FDT_NOP)
        {
            callback(token_type,0,0,0);
        }else if(token_type == FDT_END)
        {
            callback(token_type,0,0,0);
        }else
        {
            uart_puts("error type:");
            uart_puts("\n");
            return;
        }
    }

}


void dtb_callback_show_tree(uint32_t node_type, char *name, void *data, uint32_t name_size)
{
    static int level = 0;
    if(node_type==FDT_BEGIN_NODE)
    {
        for(int i=0;i<level;i++)uart_puts("   ");
        uart_puts(name);
        uart_puts("\n");
        level++;
    }else if(node_type==FDT_END_NODE)
    {
        level--;
        for(int i=0;i<level;i++)uart_puts("   ");
        uart_puts("\n");
    }else if(node_type==FDT_PROP)
    {
        for(int i=0;i<level;i++)uart_puts("   ");
        uart_puts(name);
        uart_puts("\n");

    }
}

/* http://www.wowotech.net/device_model/dt_basic_concept.html */

void dtb_callback_initramfs(uint32_t node_type, char *name, void *value, uint32_t name_size) {
    if(node_type==FDT_PROP && strcmp(name,"linux,initrd-start")==0)
    {
        
        CPIO_DEFAULT_START = (void *)(unsigned long long)uint32_endian_big2lttle(*(uint32_t*)value);
       
    }   
    if (node_type == FDT_PROP && strcmp(name, "linux,initrd-end") == 0)
    {
        CPIO_DEFAULT_END = (void *)(unsigned long long)uint32_endian_big2lttle(*(uint32_t *)value);
    }
}


void dtb_find_and_store_reserved_memory()
{
    struct fdt_header *header = (struct fdt_header *) dtb_ptr;
    if(uint32_endian_big2lttle(header->magic) != 0xD00DFEED)
    {
        uart_puts("traverse_device_tree : wrong magic in traverse_device_tree");
        return;
    }
    // off_mem_rsvmap stores all of reserve memory map with address and size
    char *dt_mem_rsvmap_ptr = (char *)((char *)header + uint32_endian_big2lttle(header->off_mem_rsvmap));
    struct fdt_reserve_entry *reverse_entry = (struct fdt_reserve_entry *)dt_mem_rsvmap_ptr;
    
    uart_printf("\033[32m[Reserve for Spin Tables]\n\033[0m");
    // reserve memory which is defined by dtb
    while (reverse_entry->address != 0 || reverse_entry->size != 0)
    {
        unsigned long long start = uint64_endian_big2lttle(reverse_entry->address);
        unsigned long long end = uint64_endian_big2lttle(reverse_entry->size) + start;
        memory_reserve(start, end);
        reverse_entry++;
    }
    uart_printf("\033[32m[Reserve for Devicetree]\n\033[0m");
    // reserve device tree itself
    memory_reserve((unsigned long long)dtb_ptr, (unsigned long long)dtb_ptr + uint32_endian_big2lttle(header->totalsize));
}

