#include "task.h"

extern list_head_t * task_head;

void add_task(task_callback_t callback, int priority) {
    task_t * new_task = (task_t *)simple_malloc(sizeof(task_t));
    new_task->prio = priority;
    new_task->callback = callback;

    list_head_t * iter;
    int not_insert_flag = 1;
    list_for_each(iter, task_head) {
        task_t * cur_task = (task_t *) list_entry(iter, task_t, listhead);
        task_t * next_task = (task_t *) list_entry(iter->next, task_t, listhead);
        if (cur_task->prio <= priority && priority <= next_task->prio) {
            not_insert_flag = 0;
            __list_add(&new_task->listhead, iter, iter->next);
            break;
        }
    }
    if (not_insert_flag) {
        list_add_tail(&new_task->listhead, task_head);
    }
}

void pop_task() {
    while (!list_empty(task_head)) {
        task_t * first = (task_t *) list_entry(task_head->next, task_t, listhead);
        list_del_entry(task_head->next);
        first->callback();
    }
}