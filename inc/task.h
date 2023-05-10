#ifndef _TASK_H
#define _TASK_H

#include <type.h>
#include <sched.h>

task_struct *task_create(void);
void task_free(task_struct *task);

#endif