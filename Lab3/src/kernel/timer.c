#include "stdlib.h"

void get_current_time()
{
    long cntpct_el0, cntfrq_el0;
    long cntp_tval_el0;
    long cntp_cval_el0;

    asm volatile(
        "mrs %0, cntpct_el0;"
        "mrs %1, cntfrq_el0;"
        "mrs %2, cntp_tval_el0;"
        "mrs %3, cntp_cval_el0;"
        : "=r"(cntpct_el0), "=r"(cntfrq_el0), "=r"(cntp_tval_el0), "=r"(cntp_cval_el0));

    long nowtime = cntpct_el0 / cntfrq_el0;

    printf("%ld seconds after booting\n", nowtime);

    // printf("cntpct_el0 = %d\n", cntpct_el0);
    // printf("cntfrq_el0 = %d\n", cntfrq_el0);
    // printf("cntp_tval_el0 = %d\n", cntp_tval_el0);
    // printf("cntp_cval_el0 = %d\n", cntp_cval_el0);

    return;
}

void el0_timer_handler(long cntpct_el0, long cntfrq_el0)
{
    // disable core timer interrupt
    asm volatile(
        "mov x1, 0;"
        "msr cntp_ctl_el0, x1;");

    long nowtime = cntpct_el0 / cntfrq_el0;
    printf("Time out, now time: %ld seconds after booting\n", nowtime);

    return;
}