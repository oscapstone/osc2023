#include "dtb.h"
#include "uart.h"
#include "utils.h"
#include "cpio.h"
#include "shell.h"

void* CPIO_DEFAULT_PLACE;
char* dtb_ptr;

//stored as big endian
struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

//we need to translate big to little endian
uint32_t uint32_endian_big2lttle(uint32_t data)
{
    char* r = (char*)&data;  
    return (r[3]<<0) | (r[2]<<8) | (r[1]<<16) | (r[0]<<24);
}


void traverse_device_tree(void *dtb_ptr, dtb_callback callback)
{
    struct fdt_header* header = dtb_ptr;
    if(uint32_endian_big2lttle(header->magic) != 0xD00DFEED)  //0xD00DFEED: magic number
    {
        uart_send_string("traverse_device_tree : wrong magic in traverse_device_tree");
        return;
    }

    uint32_t struct_size = uint32_endian_big2lttle(header->size_dt_struct);
    char* dt_struct_ptr = (char*)((char*)header + uint32_endian_big2lttle(header->off_dt_struct));
    char* dt_strings_ptr = (char*)((char*)header + uint32_endian_big2lttle(header->off_dt_strings));

    char* end = (char*)dt_struct_ptr + struct_size; //STRUCT END
    char* pointer = dt_struct_ptr;
    
    while(pointer < end)
    {
        uint32_t token_type = uint32_endian_big2lttle(*(uint32_t*)pointer);

        pointer += 4;

        if(token_type == FDT_BEGIN_NODE) 
        {
            callback(token_type, pointer, 0, 0);
            pointer += strlen(pointer); //name len
            pointer += 4 - (unsigned long long) pointer % 4;           //alignment 4 byte
        }else if(token_type == FDT_END_NODE)
        {
            callback(token_type, 0, 0, 0);
        }else if(token_type == FDT_PROP) //property tag
        {
            uint32_t len = uint32_endian_big2lttle(*(uint32_t*)pointer); //len
            pointer += 4;
            char* name = (char*)dt_strings_ptr + uint32_endian_big2lttle(*(uint32_t*)pointer); //nameoff
            pointer += 4; //data (value)
            callback(token_type, name, pointer, len);

            pointer += len; //+data size 
            if((unsigned long long)pointer % 4 != 0) pointer += 4 - (unsigned long long)pointer%4;   //alignment 4 byte

        }else if(token_type == FDT_NOP) //nop
        {
            callback(token_type, 0, 0, 0);
        }else if(token_type == FDT_END)//end
        {
            callback(token_type, 0, 0, 0);
        }else
        {
            //uart_puts("error type:%x\n",token_type);
            uart_send_string("error type");
            uart_hex(token_type);
            uart_send_string("\n");
            return;
        }
    }
}


void dtb_callback_initramfs(uint32_t node_type, char *name, void *value, uint32_t name_size) {
    
    if(node_type==FDT_PROP && strcmp(name,"linux,initrd-start")==0)
    {
        CPIO_DEFAULT_PLACE = (void *)(unsigned long long)uint32_endian_big2lttle(*(uint32_t*)value);
    }
}
