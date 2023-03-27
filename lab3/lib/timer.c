#include "timer.h"

void core_timer_enable(void) {
    asm volatile(
        "mov    x0, 1\r\n\t"                // enable
        "msr    cntp_ctl_el0, x0\r\n\t"
        "mrs    x0, cntfrq_el0\r\n\t"
        "msr    cntp_tval_el0, x0\r\n\t"    // set expired time
        "mov    x0, 2\r\n\t"
        "ldr    x1, =0x40000040\r\n\t"      // CORE0_TIMER_IRQ_CTRL
        "str    w0, [x1]\r\n\t"             // unmask timer interrupt
    );
}

void core_timer_handler(void) {
    asm volatile(
        "mrs    x0, cntfrq_el0\r\n\t"
        "add    x0, x0, x0\r\n\t"
        "msr    cntp_tval_el0, x0\r\n\t"
    );
}

unsigned long get_current_time(void) {
    unsigned long current;
    asm volatile("mrs %0, cntpct_el0\r\n" :"=r"(current) ::"memory");
    return current;
}

unsigned long get_core_frequency(void) {
    unsigned long frequency;
    asm volatile("mrs %0, cntfrq_el0\r\n" :"=r"(frequency) ::"memory");
    return frequency;
}