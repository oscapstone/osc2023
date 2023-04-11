#include "uart.h"
#include "shell.h"
#include "dtb.h"
#include "exception.h"
#include "utils.h"
#include "timer.h"
#include "mm.h"


char* dtb_ptr;
int kernel(char* arg)
{
    uart_init();

    dtb_ptr = arg;
    traverse_device_tree(dtb_ptr, dtb_callback_initramfs);
    
    init_allocator();

    // set up serial console
    uart_init();
    

    irqtask_list_init();
    timer_list_init();
   
    core_timer_enable();
    //uart_interrupt_enable();
    el1_interrupt_enable();  // enable interrupt in EL1 -> EL1
    


    // Welcome Message
    welcome_msg();
    
    // start shell
    shell_start();

    return 0;
}