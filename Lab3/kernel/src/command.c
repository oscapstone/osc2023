#include "shell.h"
#include "string.h"
#include "uart.h"
#include "dtb.h"
#include "cpio.h"
#include "mailbox.h"
#include "uart_boot.h"
#include "malloc.h"
#include "vt.h"
#include "timer.h"




void command_not_found (char * s) {

    uart_async_puts("Err: command ");
    // uart_async_puts(s);
    uart_async_puts(" not found, try <help>\n");

}

extern void* CPIO_DEFAULT_PLACE;
void command_help (){

    /* Lab1 Feature */
    uart_async_puts("help\t:  Print this help menu.\n");
    uart_async_puts("hello\t:  Print \"Hello World!\".\n");
    uart_async_puts("info\t:  Print \"Board Revision\".\n");
    uart_async_puts("reboot\t:  Reboot the device.\n");
    
    /* Lab2 Feature */
    uart_async_puts("ls\t:  List all file.\n");
    uart_async_puts("cat\t:  Cat the file.\n");
    uart_async_puts("dtb\t:  Show device tree.\n");

    /* Lab3 Feature */
    uart_async_puts("exec\t: Execute a command, replacing current image with a new image.\n");
    uart_async_puts("el\t:  Show the exception level.\n");
    uart_async_puts("setTimeout\t: setTimeout [MESSAGE] [SECONDS].\n");
    uart_async_puts("set2s\t: set 2s Alert.\n");

}

void command_hello (){

    uart_async_puts("Hello World!\n");

}



void command_reboot (){

    uart_async_puts("Start Rebooting...\n");
    *PM_WDOG = PM_PASSWORD | 0x20;
    *PM_RSTC = PM_PASSWORD | 100;
	while(1);

}

void command_info()
{
    char str[20];  
    uint32_t board_revision = mbox_get_board_revision ();
    uint32_t arm_mem_base_addr;
    uint32_t arm_mem_size;
    mbox_get_arm_memory_info(&arm_mem_base_addr, &arm_mem_size);
    
    uart_async_puts("Board Revision: ");
    if ( board_revision ){

        itohex_str(board_revision, sizeof(uint32_t), str);
        uart_async_puts(str);
        uart_async_puts("\n");

    }
    
    else{

        uart_async_puts("Unable to query serial!\n");

    }

    uart_async_puts("ARM memory base address: ");
    if ( arm_mem_base_addr ){

        itohex_str(arm_mem_base_addr, sizeof(uint32_t), str);
        uart_async_puts(str);
        uart_async_puts("\n");

    }

    else{

        uart_async_puts("0x00000000\n");

    }

    uart_async_puts("ARM memory size: ");
    if ( arm_mem_size ){

        itohex_str( arm_mem_size, sizeof(uint32_t), str);
        uart_async_puts(str);
        uart_async_puts("\n");

    }

    else{

        uart_async_puts("0x00000000\n");
    }
    
}



void command_clear (){

    uart_async_puts("\033[2J\033[H");

}





void command_ls(){

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
            uart_async_puts("cpio parse error");
            break;
        }

        //if this is not TRAILER!!! (last of file)
        if(header_ptr!=0) {
            uart_async_puts(c_filepath);
            uart_async_puts("\n");
            
        };
    }
}


void command_cat(char *args){

    char* c_filepath;
    char* c_filedata;
    unsigned int c_filesize;
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_PLACE;

    while(header_ptr!=0){
        int error = cpio_newc_parse_header(header_ptr, &c_filepath, &c_filesize, &c_filedata, &header_ptr);
        //if parse header error
        if(error){
            uart_async_puts("cpio parse error");
            break;
        }

        if(strcmp(c_filepath, args)==0){
            uart_async_puts(c_filedata);
            uart_async_puts("\n");
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_ptr==0) {
            uart_async_puts("cat:");
            uart_async_puts(args);
            uart_async_puts("No such file or directory\n");
        }
    }
}




void *base = (void *) DT_ADDR;
void command_dtb(){
    traverse_device_tree( base, dtb_callback_show_tree);
}

#define USTACK_SIZE 0x10000
void command_exec(char *filepath){
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
            uart_async_puts("cpio parse error");
            break;
        }

        if(strcmp(c_filepath, filepath)==0)
        {
            //exec c_filedata
            char* ustack = kmalloc(USTACK_SIZE);
            
            asm("msr elr_el1, %0\n\t"
                "mov x1, 0x3c0\n\t"
                "msr spsr_el1, xzr\n\t"
                "msr sp_el0, %1\n\t"    // enable interrupt in EL0. You can do it by setting spsr_el1 to 0 before returning to EL0.
                "eret\n\t"
                :: "r" (c_filedata),
                   "r" (ustack+USTACK_SIZE));
            free(ustack);
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_ptr==0) uart_printf("cat: %s: No such file or directory\n", filepath);
    }
}


void command_el(){
    print_el();
}

void command_setTimeout(char *args){
    char* sec = str_SepbySpace(args);
    uart_printf("setTimeout: %s , %s\n", args, sec);
    
    add_timer(uart_puts,atoi(sec),args);
 
}



void command_set2sAlert()
{
    add_timer(two_second_alert, 2, "two_second_alert");
}