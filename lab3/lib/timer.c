#include "mem.h"
#include "timer.h"
#include "muart.h"
#include "utils.h"
#include "exception.h"

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

void add_core_timer(void (*func)(void*), void *args, unsigned int time) {
    struct timer_event *timer = (struct timer_event*) simple_alloc(sizeof(struct timer_event));

    timer->args     = args;
    timer->expired  = get_core_current_count() + time;
    timer->callback = func;
    timer->next     = 0;

    int update = 0;

    if (!timerlist) {
        timerlist = timer; update = 1;
    } else if (timerlist->expired > timer->expired) {
        timer->next = timerlist;
        timerlist = timer;
        update = 1;
    } else {
        struct timer_event *current = timerlist;
        while (!current->next && current->expired < timer->expired) {
            current = current->next;
        }

        timer->next = current->next;
        current->next = timer;
    }

    if (update) {
        set_core_timer(time);
    }
}

void set_core_timer(unsigned int time) {
    asm volatile("msr cntp_tval_el0, %0\r\n" :"=r"(time));
}

void remove_core_timer(void) {
    struct timer_event *timer = timerlist;
    timer->callback(timer->args);
    timerlist = timerlist->next;

    if (!timerlist) {
        core_timer_disable();
    } else {
        unsigned int count = get_core_current_count();
        set_core_timer(timerlist->expired - count);
    }
}

void print_elapsed_time(void) {
    mini_uart_puts("received core timer interrupt!\r\n");

    unsigned long frequency = get_core_frequency();
    unsigned long current   = get_core_current_count();

    mini_uart_puts("seconds after booting: ");
    printdec(current / frequency);
    mini_uart_puts("\r\n\r\n");

    set_core_timer(2 * get_core_frequency());
}

void print_core_timer_message(void *args) {
    mini_uart_puts((char*) args);
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