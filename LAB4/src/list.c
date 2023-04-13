#include "list.h"

// circular doubly linked list
void list_init(list *node)
{
    node->next = node;
    node->prev = node;
}

// 插入到原本頭節點的後一個
void insert_head(list *head, list *v)
{
    v->next = head->next;
    v->prev = head;
    head->next->prev = v;
    head->next = v;
}
// 插入到原本尾節點的前一個
void insert_tail(list *tail, list *v)
{
    v->next = tail;
    v->prev = tail->prev;
    tail->prev->next = v;
    tail->prev = v;
}

// 在此實作，刪除一律只改變兩個pointer，被刪除的節點之next,prev pointer我不把它主動設成null

// 刪除原本頭節點的後一個
list *remove_head(list *head)
{
    list *ptr;
    ptr = head->next;
    head->next = head->next->next;
    head->next->prev = head; // 注意 head->next 已經 = head->next->next;了

    return ptr;
}

// 刪除原本尾節點的前一個
list *remove_tail(list *tail)
{
    list *ptr;
    ptr = tail->prev;
    tail->prev = tail->prev->prev;
    tail->prev->next = tail;

    return ptr;
}

// 刪除任意個節點
void remove(list *node)
{
    list *next, *prev;
    next = node->next;
    prev = node->prev;

    next->prev = prev;
    prev->next = next;
}