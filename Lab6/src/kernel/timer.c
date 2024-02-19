#include "stdlib.h"
#include "timer.h"
#include "thread.h"

extern void enable_interrupt();
extern void disable_interrupt();
extern task_struct *get_current();

typedef struct timer_queue_node
{
    long second_ticks;
    char message[MESSAGE_BUFFER];
} tq;

tq timer_queue[10];
int timer_queue_front = 0;
int timer_queue_back = 0;

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

void add_timer(int sec, char *mes)
{
    get_current_time();

    for (int i = 0; i < MESSAGE_BUFFER; i++)
        timer_queue[timer_queue_back].message[i] = mes[i];

    // transfer sec to frq and store to node.second
    asm volatile(
        "msr DAIFSet, 0xf;"
        "mrs x3, cntfrq_el0;"
        "mrs x4, cntpct_el0;"
        "mov x2, %1;" // after secs seconds later will interrupt
        "mul x2, x2, x3;"
        "add x2, x2, x4;"
        "mov %0, x2;"
        : "=r"(timer_queue[timer_queue_back].second_ticks)
        : "r"(sec)
        : "x0", "x1", "x2", "x3", "x4", "memory"); // Uses register operands x0, x1, x2, x3, and x4 and specifies that the instruction may modify memory using the clobber list "x0", "x1", "x2", "x3", "x4", "memory".

    timer_queue_back++;
    // Find min
    for (int i = timer_queue_front; i < timer_queue_back; i++)
    {
        if (timer_queue[i].second_ticks < timer_queue[timer_queue_front].second_ticks)
        {
            int sec_tmp;
            char mes_tmp[MESSAGE_BUFFER];

            sec_tmp = timer_queue[timer_queue_front].second_ticks;
            timer_queue[timer_queue_front].second_ticks = timer_queue[i].second_ticks;
            timer_queue[i].second_ticks = sec_tmp;

            for (int j = 0; j < MESSAGE_BUFFER; j++)
            {
                mes_tmp[j] = timer_queue[timer_queue_front].message[j];
                timer_queue[timer_queue_front].message[j] = timer_queue[i].message[j];
                timer_queue[i].message[j] = mes_tmp[j];
            }
        }
    }

    asm volatile(
        "msr cntp_cval_el0, %0;"
        "bl core_timer_enable;"
        "msr DAIFClr, 0xf;"
        :
        : "r"(timer_queue[timer_queue_front].second_ticks));
}

void el0_timer_handler(long cntpct_el0, long cntfrq_el0)
{
    disable_interrupt();
    schedule();
    enable_interrupt();
}

void el1_timer_handler(long cntpct_el0, long cntfrq_el0)
{
    if (!is_timer_queue_empty())
    {
        // disable core timer interrupt
        asm volatile(
            "mov x1, 0;"
            "msr cntp_ctl_el0, x1;");

        printf("\n");
        long nowtime = cntpct_el0 / cntfrq_el0;
        printf("Time out, now time: %ld seconds after booting\n", nowtime);
        printf("Message: %s\n", timer_queue[timer_queue_front].message);

        timer_queue_front++;
        // Find min
        for (int i = timer_queue_front; i < timer_queue_back; i++)
        {
            if (timer_queue[i].second_ticks < timer_queue[timer_queue_front].second_ticks)
            {
                int sec_tmp;
                char mes_tmp[MESSAGE_BUFFER];

                sec_tmp = timer_queue[timer_queue_front].second_ticks;
                timer_queue[timer_queue_front].second_ticks = timer_queue[i].second_ticks;
                timer_queue[i].second_ticks = sec_tmp;

                for (int j = 0; j < MESSAGE_BUFFER; j++)
                {
                    mes_tmp[j] = timer_queue[timer_queue_front].message[j];
                    timer_queue[timer_queue_front].message[j] = timer_queue[i].message[j];
                    timer_queue[i].message[j] = mes_tmp[j];
                }
            }
        }
        asm volatile(
            "msr cntp_cval_el0, %0\n\t" // set expired time
            "bl core_timer_enable\n\t"
            :
            : "r"(timer_queue[timer_queue_front].second_ticks)
            :);
    }
    else
    {
        disable_interrupt();
        schedule();
        enable_interrupt();
    }
    return;
}

int is_timer_queue_empty()
{
    return timer_queue_front == timer_queue_back ? 1 : 0;
}

void timer_delay(long seconds)
{
    long cntpct_el0, cntfrq_el0, nowtime, due;

    asm volatile(
        "mrs %0, cntpct_el0\n\t"
        "mrs %1, cntfrq_el0\n\t"
        : "=r"(cntpct_el0), "=r"(cntfrq_el0)::);

    due = cntpct_el0 / cntfrq_el0 + seconds;
    nowtime = cntpct_el0 / cntfrq_el0;

    while (nowtime <= due)
    {
        asm volatile(
            "mrs %0, cntpct_el0\n\t"
            "mrs %1, cntfrq_el0\n\t"
            : "=r"(cntpct_el0), "=r"(cntfrq_el0)::);
        nowtime = cntpct_el0 / cntfrq_el0;
    }
}