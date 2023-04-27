#include "ds/list.h"
#include "mem/mem.h"

void ds_list_head_init(struct ds_list_head *head) {
    head->next = head;
    head->prev = head;
}
void ds_list_addnext(struct ds_list_head *base, struct ds_list_head *next) {
    next->next = base->next;
    next->prev = base;
    base->next->prev = next;
    base->next = next;
}

void ds_list_addprev(struct ds_list_head *base, struct ds_list_head *prev) {
    prev->prev = base->prev;
    prev->next = base;
    base->prev->next = prev;
    base->prev = prev;
}

void ds_list_remove(struct ds_list_head *head) {
    struct ds_list_head *next = head->next;
    struct ds_list_head *prev = head->prev;
    next->prev = prev;
    prev->next = next;
}

struct ds_list_head * ds_list_front(struct ds_list_head *head) {
    if(head->next == head) return NULL;
    return head->next;
}