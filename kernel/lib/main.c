#include "uart.h"
#include "mailbox.h"
#include "shell.h"
#include "string.h"
#include "malloc.h"
#include "timer.h"
#include "dtb.h"
#include "cpio.h"
#include "exception.h"
#include "task.h"
#include "mm.h"

extern char* dtb_base;

void init()
{
    task_list_init();
    timer_list_init();
    core_timer_enable();
    core_timer_interrupt_enable();
    uart_init();
    enable_mini_uart_interrupt();
    enable_interrupt();
}

void main(char* dtb)
{
    dtb_base = dtb;
    
    init();
    
    traverse_device_tree(dtb_callback_initramfs);

    memory_init();
    shell();
}