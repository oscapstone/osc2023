#include "uart.h"
#include "shell.h"
#include "hardware_info.h"
#include "dtb.h"
#include "utils.h"
#include "initramfs.h"
#include "note.h"
#include "time_interrupt.h"
#include "exception.h"
#include "mm.h"
extern char *_dtb_ptr;
extern char _stext;
extern char _etext;
extern void core_timer_enable();
extern void enable_local_all_interrupt();
int main(void)
{
    int el = get_el();
    set_exception_vector_table();
    
    uart_init();
    uart_flush();
    uart_write_string("Hello kernel!\n");
    uart_write_string("kernel run in exception level: ");
    uart_write_no(el);
    uart_write_string("\n");
    write_uptime();
    uart_write_string("device tree address: ");
    uart_write_no_hex((unsigned int)_dtb_ptr);
    uart_write_string("\n");
    int fdt_ret = fdt_init(&_fdt, (unsigned int)_dtb_ptr);
    if (fdt_ret) {
        uart_write_string("fail on fdt_init!\n");
    } else {
        uart_write_string("fdt inited!\n");
        if (_fdt.fdt_traverse(&_fdt, initramfs_fdt_cb, NULL)) {
            uart_write_string("fdt traverse fail!\n");
        }
        uart_write_string("found cpio address!\n");
    }
    dump_hex(&cpio_addr, 8);
    init_initramfs(&_initramfs);
    //meory init
    init_buddy(&_buddy);
    init_mem_pool(&_mem_pool);

    init_note();
    print_hw_info();
    uart_write_string("text section starts at: ");
    uart_write_no_hex((unsigned long long)&_stext);
    uart_write_string("\n");
    uart_write_string("text section ends at: ");
    uart_write_no_hex((unsigned long long)&_etext);
    uart_write_string("\n");
    init_interrupt_scheduler(&_interrupt_scheduler);
    timer_task_scheduler_init(&_timer_task_scheduler);
    // core_timer_enable();
    enable_local_all_interrupt();
    //test
    // print_uptime_every2second();
    init_idle_thread();
    size_t freq = get_timer_freq();
    _timer_task_scheduler.interval_run_tick(&_timer_task_scheduler, (timer_interrupt_callback_t)time_reschedule, NULL, freq >> 5);
    // _timer_task_scheduler.interval_run_tick(&_timer_task_scheduler, (timer_interrupt_callback_t)write_uptime, NULL, freq >> 5);

    create_thread(shell_main_thread);
    idle_thread();
    return 0;
}
