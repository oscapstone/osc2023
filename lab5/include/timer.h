#include "list.h"

struct user_timer
{
    struct list_head list;

    unsigned long execution_time;
    unsigned long current_system_time;
    unsigned int trigger_time;

    char message[50];
};

void timer_router(unsigned long cntpct, unsigned long cntfrq);

void print_timestamp(unsigned long cntpct, unsigned long cntfrq);

void init_timer();
void handle_due_timeout();
