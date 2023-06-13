#ifndef TASK_H
#define TASK_H

#include "list.h"

#define LOW_PRIORITY 100

typedef void (*task_callback_t)(void);

typedef struct task
{
    list_head_t listhead;
    int prio;
    task_callback_t callback;
} task_t;

void add_task(task_callback_t callback, int priority);
void pop_task();

#endif