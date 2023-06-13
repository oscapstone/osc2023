#include "uart.h"
#include "shell.h"
#include "dtb.h"

#include "string.h"

int kernel()
{
    


    void *base = (void *) DT_ADDR;
    traverse_device_tree(base, dtb_callback_initramfs);
    

    //simple allocator
    //malloc(0x10);


    // set up serial console
    uart_init();
    // Welcome Message
    welcome_msg();




    // start shell
    shell_start();

    return 0;
}