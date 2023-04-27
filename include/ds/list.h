#ifndef __DS_LIST_H
#define __DS_LIST_H
#include "type.h"

struct ds_list_head {
    struct ds_list_head *next, *prev;
};

void ds_list_head_init(struct ds_list_head *head);
void ds_list_addnext(struct ds_list_head *base, struct ds_list_head *next);
void ds_list_addprev(struct ds_list_head *base, struct ds_list_head *prev);
void ds_list_remove(struct ds_list_head *head);
struct ds_list_head * ds_list_front(struct ds_list_head*);


#endif