#include "test/demo.h"
#include "test/test_kmalloc.h"
#include "test/demo_page.h"
#include "test/test_random.h"
#include "peripherals/mini_uart.h"
#include "event.h"
#include "type.h"

/* this is cursed code :(*/


struct k_event demo_timer_event;
uint32_t inf_loop;
void demo_timer_interrupt() {
    unsigned long time, freq;
    asm volatile("mrs %0, cntfrq_el0":"=r"(freq));
    asm volatile("mrs %0, cntpct_el0":"=r"(time));
    // set another timeout
    asm volatile(
        "mrs x0, cntfrq_el0\n"
        "add x0, x0, x0\n"
        "msr cntp_tval_el0, x0\n"
    );
    uart_send_string("Seconds after boot: ");
    // get seconds from freq
    uart_send_dec(time / freq);
    uart_send_string("\r\n");
}
void demo_preempt(void *ptr, uint32_t sz) {
    uart_send('h');
}
void demo_preempt_vic(void *ptr, uint32_t sz) {
    while(1){asm volatile("nop");}
}
void demo_init() {
    // k_event_init(&demo_timer_event, &demo_timer_interrupt);
    k_event_init(&demo_timer_event, &demo_preempt_vic);
}