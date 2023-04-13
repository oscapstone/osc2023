#ifndef TASK_H
#define TASK_H

#include "list.h"

#define UART_IRQ_PRIORITY 1
#define TIMER_IRQ_PRIORITY -1

typedef void (*task_callback_t)(void);

typedef struct task
{
    list_head_t listhead;
    int nice;
    task_callback_t callback;
} task_t;

void add_task(task_callback_t callback, int nice);
void pop_task();
void test_preemption();

#endif