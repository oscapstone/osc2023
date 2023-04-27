#include "peripherals/rpi_uart.h"
#include "peripherals/rpi_mailbox.h"
#include "shell.h"
#include "utils.h"
#include "uart.h"
#include "dtb.h"
#include "cpio.h"
#include "uart_boot.h"
#include "mm.h"
#include "vt.h"
#include "timer.h"

extern char* dtb_ptr;
extern int   uart_recv_echo_flag;


void command_not_found (char * s) {

    uart_puts("Err: command ");
    // uart_puts(s);
    uart_puts(" not found, try <help>\n");

}

extern void* CPIO_DEFAULT_START;
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
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_START;

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
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_START;

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





void command_dtb(){
    traverse_device_tree( dtb_ptr, dtb_callback_show_tree);
}

#define USTACK_SIZE 0x10000
void command_exec(char *filepath){
    char* c_filepath;
    char* c_filedata;
    unsigned int c_filesize;
    struct cpio_newc_header *header_ptr = CPIO_DEFAULT_START;

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
            uart_recv_echo_flag = 0; // syscall.img has different mechanism on uart I/O.
            exec_thread(c_filedata, c_filesize);
        }

        //if this is TRAILER!!! (last of file)
        if(header_ptr==0) uart_printf("cat: %s: No such file or directory\n", filepath);
    }

}


void command_el(){
    print_el();
}




void command_memory_tester()
{
    char *a = kmalloc(0x10);
    char *b = kmalloc(0x100);
    char *c = kmalloc(0x1000);

    kfree(a);
    kfree(b);
    kfree(c);

    a = kmalloc(32);
    char *aa = kmalloc(50);
    b = kmalloc(64);
    char *bb = kmalloc(64);
    c = kmalloc(128);
    char *cc = kmalloc(129);
    char *d = kmalloc(256);
    char *dd = kmalloc(256);
    char *e = kmalloc(512);
    char *ee = kmalloc(999);

    char *f = kmalloc(0x2000);
    char *ff = kmalloc(0x2000);
    char *g = kmalloc(0x2000);
    char *gg = kmalloc(0x2000);
    char *h = kmalloc(0x2000);
    char *hh = kmalloc(0x2000);

    kfree(a);
    kfree(aa);
    kfree(b);
    kfree(bb);
    kfree(c);
    kfree(cc);
    kfree(dd);
    kfree(d);
    kfree(e);
    kfree(ee);

    kfree(f);
    kfree(ff);
    kfree(g);
    kfree(gg);
    kfree(h);
    kfree(hh);
    
}