#ifndef __WAITQUEUE_H
#define __WAITQUEUE_H

#include "thread.h"
#include "ds/list.h"
struct waitqueue_t {
    struct ds_list_head th_list;
};

void waitqueue_init(struct waitqueue_t *queue);
void wait(struct waitqueue_t *queue);

#endif