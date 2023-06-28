#ifndef OSCOS_XCPT_TASK_QUEUE_H
#define OSCOS_XCPT_TASK_QUEUE_H

#include <stdbool.h>

bool task_queue_add_task(void (*task)(void *), void *arg, int priority);

#endif
