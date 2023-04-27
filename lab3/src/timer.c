#include "timer.h"
#include "mini_uart.h"
#include "utils.h"
#include "mem.h"

static struct timer_event *timerlist = 0;

void infinite(void) {
    while (1);
}

void core_timer_enable(void) {
    asm volatile(
        "mov    x0, 1\r\n\t"
        "msr    cntp_ctl_el0, x0\r\n\t"     // enable
        "mov    x0, 2\r\n\t"
        "ldr    x1, =0x40000040\r\n\t"      // CORE0_TIMER_IRQ_CTRL
        "str    w0, [x1]\r\n\t"             // unmask timer interrupt
    );
}

void core_timer_disable(void) {
    asm volatile(
        "mov    x0, 0\r\n\t"
        "msr    cntp_ctl_el0, x0\r\n\t"     // disable
        "mov    x0, 0\r\n\t"
        "ldr    x1, =0x40000040\r\n\t"      // CORE0_TIMER_IRQ_CTRL
        "str    w0, [x1]\r\n\t"             // unmask timer interrupt
    );
}


void set_timer(unsigned int count){
    asm volatile("msr cntp_tval_el0, %0\r\n" :"=r"(count));
}

void print_boottime(void){
    unsigned long count = get_core_current_count();
    unsigned long freq = get_core_frequency();

    uart_puts("After booting: ");
    uart_dec(count / freq);
    uart_puts(" seconds\r\n\r\n");

    set_timer(2*freq);
}

void add_core_timer(const char *message, unsigned int time) {
    struct timer_event *timer = (struct timer_event*) simple_malloc(sizeof(struct timer_event));

    strcpy(timer->message, message);
    timer->expired  = get_core_current_count() + time * get_core_frequency();
    timer->next     = 0;

    int ishead = 0;

    if (timerlist == 0) {   // Head
        timerlist = timer; 
        ishead = 1;
    } 
    else if (timerlist->expired > timer->expired) {   // Insert to head
        timer->next = timerlist;
        timerlist = timer;
        ishead = 1;
    } 
    else {    // Insert to mid or tail
        struct timer_event *current = timerlist;
        while (current->next && current->next->expired < timer->expired) {
            current = current->next;
        }

        timer->next = current->next;
        current->next = timer;
    }

    if (ishead) {
        set_timer(time * get_core_frequency());
    }
}

void core_timer_handler(void) {
    struct timer_event *timer = timerlist;

    uart_puts(timer->message);

    // Call next timer
    timerlist = timerlist->next;

    if (!timerlist) {
        core_timer_disable();
    } else {
        unsigned int count = get_core_current_count();
        set_timer(timerlist->expired - count);
    }
}

unsigned long get_core_frequency(void) {
    unsigned long frequency;
    asm volatile("mrs %0, cntfrq_el0\r\n" :"=r"(frequency) ::"memory");
    return frequency;
}

unsigned long get_core_current_count(void) {
    unsigned long count;
    asm volatile("mrs %0, cntpct_el0\r\n" :"=r"(count) ::"memory");
    return count;
}