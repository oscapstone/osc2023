#ifndef OSCOS_XCPT_TASK_QUEUE_H
#define OSCOS_XCPT_TASK_QUEUE_H

#include <stdbool.h>

bool task_queue_add(int priority, void (*task_fn)(void *), void *arg);

#endif
