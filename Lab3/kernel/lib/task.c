#include "task.h"

extern list_head_t * task_head;
int cur_priority = LOW_PRIORITY;

void add_task(task_callback_t callback, int priority) {
    task_t * new_task = (task_t *)simple_malloc(sizeof(task_t));
    new_task->prio = priority;
    new_task->callback = callback;

    disable_interrupt();
    list_head_t * iter;
    int not_insert_flag = 1;
    list_for_each(iter, task_head) {
        task_t * cur_task = (task_t *) list_entry(iter, task_t, listhead);
        if (priority < cur_task->prio) {
            not_insert_flag = 0;
            list_add(&new_task->listhead, iter->prev);
            break;
        }
    }
    if (not_insert_flag) {
        list_add_tail(&new_task->listhead, task_head);
    }
    enable_interrupt();
}

void pop_task() {
    while (!list_empty(task_head)) {
        task_t * first = (task_t *) list_entry(task_head->next, task_t, listhead);
        if (first->prio > cur_priority) {
            return;
        }

        disable_interrupt();
        list_del_entry(task_head->next);
        cur_priority = first->prio;
        enable_interrupt();

        first->callback();

        disable_interrupt();
        cur_priority = LOW_PRIORITY;
        enable_interrupt();
    }
}