#include "timer.h"
#include "../include/sched.h"
#include "mini_uart.h"


void timer_init() {
    core_timer_enable();
    set_timer(read_freq());
}

void handle_timer_irq() {
    //printf("Timer interrupt.\n");
    set_timer(read_freq()>>5);
    timer_tick();
}

void core_timer_enable() {

    asm volatile(
        "mov x0, 1\n\t"
        "msr cntp_ctl_el0, x0\n\t" // enable; set cntp_ctl_el0 to 1
        "mov x0, 2\n\t"
        "ldr x1, =0x40000040\n\t" // CORE0_TIMER_IRQ_CTRL
        "str w0, [x1]\n\t" // unmask timer interrupt
    );

}

void core_timer_disable() {

    asm volatile(
        "mov x0, 0\n\t"
        "ldr x1, =0x40000040\n\t"
        "str w0, [x1]\n\t"
    );

}


void set_timer(unsigned int rel_time) {

    asm volatile(
        "msr cntp_tval_el0, %0\n\t"
        :
        : "r" (rel_time)
    );

}

unsigned int read_timer() {

    unsigned int time;
    asm volatile("mrs %0, cntpct_el0\n\t" : "=r" (time) :  : "memory");
    return time;

}

unsigned int read_freq() {

    unsigned int freq;
    asm volatile("mrs %0, cntfrq_el0\n\t": "=r" (freq) : : "memory");
    return freq;

}