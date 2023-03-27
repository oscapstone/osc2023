#include "uart.h"
#include "shell.h"
#include "dtb.h"
#include "exception.h"
#include "string.h"
#include "timer.h"

int kernel()
{
    


    void *base = (void *) DT_ADDR;
    traverse_device_tree(base, dtb_callback_initramfs);
    

    //simple allocator
    //malloc(0x10);

    
    // set up serial console
    uart_init();


    //irqtask_list_init();
    //timer_list_init();

    //uart_interrupt_enable();
    //el1_interrupt_enable();  // enable interrupt in EL1 -> EL1
    //core_timer_enable();


    // Welcome Message
    welcome_msg();
    // start shell
    shell_start();

    return 0;
}