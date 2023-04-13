#include "task.h"
#include "exception.h"
#include "string.h"
#include "malloc.h"
#include "uart.h"
#include "timer.h"

int curr_task_nice = 123456; 

list_head_t task_list;

void task_list_init()
{
    INIT_LIST_HEAD(&task_list);
}

void add_task(task_callback_t callback, int nice){
    task_t *the_task = smalloc(sizeof(task_t));

    the_task->nice = nice;
    the_task->callback = callback;
    INIT_LIST_HEAD(&the_task->listhead);

    list_head_t *curr;

    disable_interrupt(); 
    list_for_each(curr, &task_list)
    {
        if (((task_t *)curr)->nice > the_task->nice)
        {
            list_add(&the_task->listhead, curr->prev);
            break;
        }
    }

    if (list_is_head(curr, &task_list))
        list_add_tail(&the_task->listhead, &task_list); 
    
    enable_interrupt();
}

void pop_task(){
    while (!list_empty(&task_list))
    {
        disable_interrupt();
        task_t *the_task = (task_t *)task_list.next;
        if (curr_task_nice <= the_task->nice) // curr task has higher priority
        {
            enable_interrupt();
            break;
        }
        list_del_entry((list_head_t *)the_task);
        int prev_task_nice = curr_task_nice;
        curr_task_nice = the_task->nice;
        enable_interrupt();

        the_task->callback();

        disable_interrupt();
        curr_task_nice = prev_task_nice;
        enable_interrupt();
    }
}

void highp()
{
    uart_async_printf("high prior start\n");
    uart_async_printf("high prior end\n");
}

void lowp()
{
    uart_async_printf("\nlow prior start\n");
    add_task(highp, 0);
    //add_timer(uart_puts, "low prior end\n", 5);  // Buggy
    uart_async_printf("\r"); // to trigger pop_task
    for (int i = 0; i < 1000000; ++i);
    uart_async_printf("low prior end\n");
}

void test_preemption()
{
    add_task(lowp, 9);
}