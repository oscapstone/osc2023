#include "dtb.h"
#include "uart.h"
#include "timer.h"
#include "task.h"
#include "shell.h"
#include "list.h"
#include "memory.h"
#include "interrupt.h"

extern char* dtb_base;

/* task */
LIST_HEAD(task_head_item);
list_head_t * task_head = &task_head_item;

void main(char *arg)
{
    dtb_base = arg;

    fdt_traverse(initramfs_callback);   // find cpio

    INIT_LIST_HEAD(task_head);
    
    init_uart_async_index();
    uart_init();
    init_memory();
    init_timer_list();
    set_cpu_timer_up();
    core_timer_interrupt_disable_alternative();
    core_timer_interrupt_enable();
    enable_uart_interrupt();
    enable_interrupt();
    init_sched_thread();
    enable_core_timer();

    /* Lab 5 */
    exec_file("syscall.img");

    // shell();

}