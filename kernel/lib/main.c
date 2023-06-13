#include "dtb.h"
#include "uart.h"
#include "irq.h"
#include "cpio.h"
#include "list.h"
#include "timer.h"
#include "task.h"
#include "sched.h"
#include "mmu.h"
#include "mbox.h"
#include "filesystem/vfs.h"

extern char *dtb_base;



void main(char *arg)
{
    dtb_base = PHYS_TO_VIRT(arg);
    // init list
    init_idx();
    uart_init();
    task_list_init();
    fdt_traverse(initramfs_callback);
    init_allocator();
    init_rootfs();
    timer_list_init();
   
    enable_interrupt();
    enable_mini_uart_interrupt();
   
    init_thread_sched();
   
    core_timer_enable();

    
  

    uart_printf("Lab7 :\n");

    //clear();
    execfile("vfs1.img");
    //shell();
}