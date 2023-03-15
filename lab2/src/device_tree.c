#include "peripherals/device_tree.h"
#include "mini_uart.h"
#include "string_utils.h"

struct fdt_header {
        char magic[4];
        unsigned int totalsize;
        unsigned int off_dt_struct;
        unsigned int off_dt_strings;
        unsigned int off_mem_rsvmap;
        unsigned int version;
        unsigned int last_comp_version;
        unsigned int boot_cpuid_phys;
        unsigned int size_dt_strings;
        unsigned int size_dt_struct;
};

struct fdt_header* device_tree_adr;
static char* dt_struct_adr;
static char* dt_strings_adr;
int get_device_tree_adr(void)
{
        asm volatile("mov %0, x28" :  "=r"(device_tree_adr));
        
        /*
         * Checks device tree magic number
         */
        char* magic = &(device_tree_adr->magic[0]);
        if (magic[0] != FDT_MAGIC_0 || magic[1] != FDT_MAGIC_1
                        || magic[2] != FDT_MAGIC_2 || magic[3] != FDT_MAGIC_3) {
                uart_send_string("[ERROR] Device tree: wrong magic\r\n");
                return 1;
        }
        
        dt_struct_adr = (char*)device_tree_adr + 
                big_bytes_to_uint((char*)&(device_tree_adr->off_dt_struct), 4);
        dt_strings_adr = (char*)device_tree_adr +
                big_bytes_to_uint((char*)&(device_tree_adr->off_dt_strings), 4);
        return 0;
}

void fdt_traverse(int (*req_callback)(char*, char*, char*, unsigned int))
{
        char* ptr = dt_struct_adr;
        char* node_name;

        while (1) {
                unsigned int token = big_bytes_to_uint(ptr, 4);
                ptr += 4;

                if (token == FDT_BEGIN_NODE) {
                        node_name = ptr;
                        /*
                         * Skips node name and aligns to 32-bit boundary
                         */
                        int offset = strlen(ptr) + 1;
                        int tmp = offset % 4;
                        offset += (4 - tmp) % 4;
                        ptr += offset;
                } else if (token == FDT_END_NODE) {
                        continue;
                } else if (token == FDT_PROP) {
                        unsigned int prop_len = big_bytes_to_uint(ptr, 4);
                        ptr += 4;

                        char* prop_name = dt_strings_adr 
                                        + big_bytes_to_uint(ptr, 4);
                        ptr += 4;

                        char* prop_value_ptr = ptr;
                        /*
                         * Skips value and aligns to 32-bit boundary
                         */
                        int offset = prop_len;
                        int tmp = offset % 4;
                        offset += (4 - tmp) % 4;
                        ptr += offset;

                        if (req_callback(node_name, prop_name, 
                                        prop_value_ptr, prop_len)) {
                                break;
                        }
                } else if (token == FDT_NOP) {
                        continue;
                } else if (token == FDT_END) {
                        break;
                } else {
                        uart_send_string("[ERROR] Device tree: unknown token ");
                        uart_send_int(token);
                        uart_endl();
                        break;
                }
        }
}