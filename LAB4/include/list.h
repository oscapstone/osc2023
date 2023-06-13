#ifndef _LIST_H_
#define _LIST_H_
#include <stdint.h>
#include <stddef.h>

// 將串接的工作抽象出來，linking linked list是為了搭配以後的container of
// 且list是circular doubly linked list
typedef struct list
{
    struct list *next, *prev;
} list;
// 另外這裡要實做的是:LIFO circular doubly linked list

void list_init(list *node);
void insert_head(list *head, list *v);
void insert_tail(list *head, list *v);
list *remove_head(list *head);
list *remove_tail(list *head);
void remove(list *node);
#endif
