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

    uart_puts("Err: command ");
    // uart_puts(s);
    uart_puts(" not found, try <help>\n");

}

extern void* CPIO_DEFAULT_PLACE;
void command_help (){

    /* Lab1 Feature */
    uart_puts("help\t:  Print this help menu.\n");
    uart_puts("hello\t:  Print \"Hello World!\".\n");
    uart_puts("info\t:  Print \"Board Revision\".\n");
    uart_puts("reboot\t:  Reboot the device.\n");
    
    /* Lab2 Feature */
    uart_puts("ls\t:  List all file.\n");
    uart_puts("cat\t:  Cat the file.\n");
    uart_puts("dtb\t:  Show device tree.\n");

    /* Lab3 Feature */
    uart_puts("exec\t: Execute a command, replacing current image with a new image.\n");
    uart_puts("el\t:  Show the exception level.\n");
    uart_puts("setTimeout\t: setTimeout [MESSAGE] [SECONDS].\n");
    uart_puts("set2sAlert\t: set core timer interrupt every 2 second.\n");


}

void command_hello (){

    uart_puts("Hello World!\n");

}



void command_reboot (){

    uart_puts("Start Rebooting...\n");
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
    
    uart_puts("Board Revision: ");
    if ( board_revision ){

        itohex_str(board_revision, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");

    }
    
    else{

        uart_puts("Unable to query serial!\n");

    }

    uart_puts("ARM memory base address: ");
    if ( arm_mem_base_addr ){

        itohex_str(arm_mem_base_addr, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");

    }

    else{

        uart_puts("0x00000000\n");

    }

    uart_puts("ARM memory size: ");
    if ( arm_mem_size ){

        itohex_str( arm_mem_size, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");

    }

    else{

        uart_puts("0x00000000\n");
    }
    
}



void command_clear (){

    uart_puts("\033[2J\033[H");

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
            uart_puts("cpio parse error");
            break;
        }

        //if this is not TRAILER!!! (last of file)
        if(header_ptr!=0) {
            uart_puts(c_filepath);
            uart_puts("\n");
            
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
            uart_puts("cpio parse error");
            break;
        }

        if(strcmp(c_filepath, args)==0){
            uart_puts(c_filedata);
            uart_puts("\n");
            break;
        }

        //if this is TRAILER!!! (last of file)
        if(header_ptr==0) {
            uart_puts("cat:");
            uart_puts(args);
            uart_puts("No such file or directory\n");
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
            uart_puts("cpio parse error");
            break;
        }

        if(strcmp(c_filepath, filepath)==0)
        {
            //exec c_filedata
            char* ustack = kmalloc(USTACK_SIZE);
            
            asm("msr elr_el1, %0\n\t"   // elr_el1: Set the address to return to: c_filedata
                "mov x1, 0x3c0\n\t"
                "msr spsr_el1, xzr\n\t" // enable interrupt (PSTATE.DAIF) -> spsr_el1[9:6]=4b0. 
                "msr sp_el0, %1\n\t"    // user program stack pointer set to new stack.
                "eret\n\t"              // Perform exception return. EL1 -> EL0
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