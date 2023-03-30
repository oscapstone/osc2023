#include <stddef.h>
#include "shell.h"
#include "uart.h"
#include "mbox.h"
#include "reboot.h"
#include "my_str.h"
#include "heap.h"
#include "cpio.h"
#include "dtb.h"

extern char* dtb_ptr;
extern void* CPIO_DEFAULT_PLACE;

static unsigned int parse_hex_str(char *s, unsigned int max_len)
{
    unsigned int r = 0;

    for (unsigned int i = 0; i < max_len; i++) {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            r += s[i] - '0';
        }  else if (s[i] >= 'a' && s[i] <= 'f') {
            r += s[i] - 'a' + 10;
        }  else if (s[i] >= 'A' && s[i] <= 'F') {
            r += s[i] - 'A' + 10;
        } else {
            return r;
        }
    }
    return r;
}


void shell_init(){
    uart_init();
    uart_flush();
    uart_printf("\nWelcome! OSC2023\n");
}
void cli_cmd_clear(char* buffer, int length)
{
    for(int i=0; i<length; i++)
    {
        buffer[i] = '\0';
    }
};
void shell_input(char *buffer){
    uart_printf("\t#");
    char c='\0';
    int idx = 0;
    while(1)
    {
        if ( idx >= CMD_MAX_LEN ) break;

        c = uart_read();
        if ( c == '\n')
        {
            uart_printf("\r\n");
            break;
        }
        buffer[idx++] = c;
        uart_write(c);
    }
    
}
void shell_controller(char *buffer){ 
    
    if (!buffer) return;

    char* cmd = buffer;
    char* argvs;
    while(1){
        if(*buffer == '\0')
        {
            argvs = buffer;
            break;
        }
        if(*buffer == ' ')
        {
            *buffer = '\0';
            argvs = buffer + 1;
            break;
        }
        buffer++;
    }
    
    if (strcmp(cmd, "") == 0){
        return;
    }
    else if (strcmp(cmd, "help") == 0){
        uart_printf("help\t: print the help menu\n");
        uart_printf("hello\t: print Hello World!\n");
        uart_printf("mailbox\t: get the hardware's info!\n");
        uart_printf("ls\t: show file\n");
        uart_printf("cat\t: concatenate files and print on the standard output\n");
        uart_printf("malloc\t: simple allocator in heap session\n");
        uart_printf("dtb\t: show device tree\n");
        uart_printf("reboot\t: reboot the device\n");        
    }
    else if (strcmp(cmd, "hello") == 0){
        uart_printf("Hello World!\n");
    }
    else if (strcmp(cmd, "mailbox") == 0){
        mbox_board_revision();
        // uart_init();
        // uart_printf("change to info\n");
        mbox_memory_info();
        // uart_printf("done\n");

    }
    else if (strcmp(cmd, "ls") == 0){
        char* c_filepath;
        char* c_filedata;
        unsigned int c_filesize;
        struct cpio_newc_header *header_ptr = CPIO_DEFAULT_PLACE;
        while(header_ptr!=0)
        {
            int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
            //if parse header error
            if(error)
            {
                uart_printf("cpio parse error");
                break;
            }

            //if this is not TRAILER!!! (last of file)
            if(header_ptr!=0) uart_printf("%s\n", c_filepath);
        }
    }
    else if (strcmp(cmd, "cat") == 0){
        char* filepath = argvs;
        char* c_filepath;
        char* c_filedata;
        unsigned int c_filesize;
        struct cpio_newc_header *header_ptr = CPIO_DEFAULT_PLACE;

        while(header_ptr!=0)
        {
            int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
            //if parse header error
            if(error)
            {
                uart_printf("cpio parse error");
                break;
            }

            if(strcmp(c_filepath, filepath)==0)
            {
                uart_printf("%s", c_filedata);
                break;
            }

            //if this is TRAILER!!! (last of file)
            if(header_ptr==0) uart_printf("cat: %s: No such file or directory\n", filepath);
        }
    }
    else if (strcmp(cmd, "malloc") == 0){
        //test malloc
        char* test1 = malloc(0x18);
        memcpy(test1,"test malloc1",sizeof("test malloc1"));
        uart_printf("%s\n", test1);

        char* test2 = malloc(0x20);
        memcpy(test2,"test malloc2",sizeof("test malloc2"));
        uart_printf("%s\n", test2);

    }
    else if (strcmp(cmd, "dtb") == 0){
        traverse_device_tree(dtb_ptr, dtb_callback_show_tree);
    }
    else if (strcmp(cmd, "reboot") == 0){
        uart_printf("Rebooting..\n");
        reset(100);
        while(1);
    }
    else{
        uart_printf("Commont not found!\n");
    }    
}
