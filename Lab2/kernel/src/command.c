#include "uart.h"
#include "mailbox.h"
#include "string.h"
#include "time.h"
#include "uart_boot.h"
#include "cpio.h"
#include "dtb_parser.h"
#include "dtb.h"

void command_not_found (char * s) 
{
    uart_puts("Err: command ");
    /*uart_puts(s);*/
    uart_puts(" not found, try <help>\n");
}

void input_buffer_overflow_message ( char cmd[] )
{
    uart_puts("Follow command: \"");
    uart_puts(cmd);
    uart_puts("\"... is too long to process.\n");

    uart_puts("The maximum length of input is 64.");
}

extern void* CPIO_DEFAULT_PLACE;
void command_help ()
{
    uart_puts("help\t:  Print this help menu.\n");
    uart_puts("hello\t:  Print \"Hello World!\".\n");
    uart_puts("info\t:  Print \"Board Revision\".\n");
    uart_puts("reboot\t:  Reboot the device.\n");
    //uart_puts("loading\t:  Loading the kernel.\n");
    uart_puts("ls\t:  List all file.\n");
    uart_puts("cat\t:  Cat the file.\n");
    uart_puts("dtb_ls\t:  List device tree.\n");
    uart_puts("dtb_cat\t:  Parse device tree.\n");
    uart_puts("dtb\t:  Show device tree.\n");
    //uart_puts("timestamp\t:  Get current timestamp.\n");
    uart_puts("CPIO_DEFAULT_PLACE:[");
    uart_puts(CPIO_DEFAULT_PLACE);
    uart_puts("]\n");

}

void command_hello ()
{
    uart_puts("Hello World!\n");
}

void command_timestamp ()
{
    unsigned long int cnt_freq, cnt_tpct;
    char str[20];

    asm volatile(
        "mrs %0, cntfrq_el0 \n\t"
        "mrs %1, cntpct_el0 \n\t"
        : "=r" (cnt_freq),  "=r" (cnt_tpct)
        :
    );

    ftoa( ((float)cnt_tpct) / cnt_freq, str, 6);

    uart_send('[');
    uart_puts(str);
    uart_puts("]\n");
}



void command_reboot ()
{
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
    if ( board_revision )
    {
        itohex_str(board_revision, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");
    }
    else
    {
        uart_puts("Unable to query serial!\n");
    }
    uart_puts("ARM memory base address: ");
    if ( arm_mem_base_addr )
    {
        itohex_str(arm_mem_base_addr, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");
    }
    else
    {
        uart_puts("0x00000000\n");
    }
    uart_puts("ARM memory size: ");
    if ( arm_mem_size )
    {
        itohex_str( arm_mem_size, sizeof(uint32_t), str);
        uart_puts(str);
        uart_puts("\n");
    }
    else
    {
        uart_puts("0x00000000\n");
    }
    
}



void command_clear (){
    uart_puts("\033[2J\033[H");
}




void command_loading (){
    loading();
}

void command_cpio_list(){
    cpio_list();
}

void command_cpio_cat(char *args){
    cpio_cat(args);
}


void command_dt_info(){
    print_dt_info();
}
void command_parse_dt(){
    parse_dt();
}

void *base = (void *) DT_ADDR;
void command_dtb()
{
    traverse_device_tree( base, dtb_callback_show_tree);
}