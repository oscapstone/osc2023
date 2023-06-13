#include "dtb.h"
#include "uart.h"
#include "timer.h"
#include "task.h"
#include "shell.h"
#include "list.h"
#include "interrupt.h"

extern char* dtb_base;

/* timer */
LIST_HEAD(timer_event_head_item);
list_head_t * timer_event_head = &timer_event_head_item;

/* task */
LIST_HEAD(task_head_item);
list_head_t * task_head = &task_head_item;

void main(char *arg)
{
    dtb_base = arg;
    fdt_traverse(initramfs_callback);   // initialize cpio

    // initialize timer event list head
    INIT_LIST_HEAD(timer_event_head);

    // initialize task list head
    INIT_LIST_HEAD(task_head);

    // initialize uart async index
    init_uart_async_index();

    enable_core_timer();
    core_timer_interrupt_disable_alternative();
    core_timer_interrupt_enable();
    uart_init();
    enable_uart_interrupt();
    enable_interrupt();

    shell();
}