#ifndef _TIMER_H
#define _TIMER_H

#define MESSAGE_BUFFER 50
#define SECONDS_BUFFER 20

void get_current_time();
void el0_timer_handler(long cntpct_el0, long cntfrq_el0);
void el1_timer_handler(long cntpct_el0, long cntfrq_el0);
void add_timer(int sec, char *mes);
int is_timer_queue_empty();

#endif /*_TIMER_H */