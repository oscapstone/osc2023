#include "timer.h"
#include "printf.h"
#include "mm.h"
#include "string.h"
#include "exception.h"
#include "syscall.h"
#include "schedule.h"

struct list_head user_timer_list;
extern int user_timer;

void timer_router(unsigned long cntpct, unsigned long cntfrq)
{
    // printf("timer interrupt!\n");
    if (user_timer)
        handle_due_timeout();
    else
    {
        // print_timestamp(cntpct, cntfrq);
        // asm volatile(
        //     "mrs x0, cntfrq_el0     \n\t"
        //     "mov x1, 5              \n\t"
        //     "mul x0, x0, x1         \n\t"
        //     "msr cntp_tval_el0, x0  \n\t"
        // );

        // disable_irq();

        int pid = get_current_task();
        struct task_struct *current = task_pool[pid];

        current->quota--;
        if (current->quota <= 0)
            current->need_schedule = 1;

        // printf("pid: %d, remaining quota: %d\n", pid, current->quota);

        asm volatile(
            "mrs x0, cntfrq_el0     \n\t"
            "mov x1, 5              \n\t"
            "lsr x0, x0, x1         \n\t"
            "msr cntp_tval_el0, x0  \n\t");
    }

    return;
}

void print_timestamp(unsigned long cntpct, unsigned long cntfrq)
{
    int timestamp = cntpct / cntfrq;
    printf("timestamp: %d\n", timestamp);
    return;
}

void init_timer()
{
    list_init_head(&user_timer_list);

    // enable_irq();
    // enable_core_timer();
}

void sys_set_timeout(struct trapframe *tf)
{
    int second = tf->x[0];
    char *message = (char *)tf->x[1];

    struct user_timer *new_timer = km_allocation(sizeof(struct user_timer));
    new_timer->trigger_time = second;
    strcpy(message, new_timer->message);

    unsigned long frequency;
    unsigned long timestamp;
    asm volatile(
        "mrs %0, cntpct_el0 \n\t"
        "mrs %1, cntfrq_el0 \n\t"
        : "=r"(timestamp), "=r"(frequency)
        :);
    unsigned long system_time = timestamp / frequency;
    new_timer->current_system_time = system_time;
    new_timer->execution_time = system_time;

    // if there is no previously set timer, then set it directly
    if (list_empty(&user_timer_list))
    {
        list_add_head(&new_timer->list, &user_timer_list);
        asm volatile(
            "mov x0, 1              \n\t"
            "msr cntp_ctl_el0, x0   \n\t"
            "msr cntp_tval_el0, %0  \n\t"
            "mov x0, 2              \n\t"
            "ldr x1, =0x40000040    \n\t"
            "str w0, [x1]           \n\t"
            :
            : "r"(frequency * new_timer->trigger_time));
    }
    else
    {
        // update the system time for each timer in the list
        for (struct user_timer *temp = (struct user_timer *)user_timer_list.next; &temp->list != &user_timer_list; temp = (struct user_timer *)temp->list.next)
        {
            temp->trigger_time -= system_time - temp->current_system_time;
            temp->current_system_time = system_time;
        }

        struct user_timer *front = (struct user_timer *)user_timer_list.next;

        // overwrite cntp_tval_el0 if the trigger time of the new timer is less than that of the current one
        if (new_timer->trigger_time < front->trigger_time)
        {
            list_crop(&front->list, &front->list);
            km_free(front);
            list_add_head(&new_timer->list, &user_timer_list);

            asm volatile(
                "mov x0, 1              \n\t"
                "msr cntp_ctl_el0, x0   \n\t"
                "msr cntp_tval_el0, %0  \n\t"
                "mov x0, 2              \n\t"
                "ldr x1, =0x40000040    \n\t"
                "str w0, [x1]           \n\t"
                :
                : "r"(frequency * new_timer->trigger_time));
        }
        // find the appropriate hole to insert the new timer
        else
        {
            struct user_timer *current = front;
            struct user_timer *next = (struct user_timer *)current->list.next;

            int entered = 0;
            while ((&next->list != &user_timer_list) && (next->trigger_time < new_timer->trigger_time))
            {
                entered = 1;
                next = (struct user_timer *)current->list.next;
                current = next;
            }
            // if the above loop is entered, go one step back
            if (entered)
                current = (struct user_timer *)current->list.prev;

            __list_add(&new_timer->list, &current->list, current->list.next);
        }
    }

    return;
}

void handle_due_timeout()
{
    unsigned long frequency;
    unsigned long timestamp;
    asm volatile(
        "mrs %0, cntpct_el0 \n\t"
        "mrs %1, cntfrq_el0 \n\t"
        : "=r"(timestamp), "=r"(frequency)
        :);
    unsigned long system_time = timestamp / frequency;

    struct user_timer *front = (struct user_timer *)user_timer_list.next;
    printf("user timer due! message: %s, current time: %d, execution time: %d\n", front->message, system_time, front->execution_time);

    list_crop(&front->list, &front->list);
    km_free(front);

    if (!list_empty(&user_timer_list))
    {
        for (struct user_timer *temp = (struct user_timer *)user_timer_list.next; &temp->list != &user_timer_list; temp = (struct user_timer *)temp->list.next)
        {
            temp->trigger_time -= system_time - temp->current_system_time;
            temp->current_system_time = system_time;
        }

        front = (struct user_timer *)user_timer_list.next;
        asm volatile(
            "msr cntp_tval_el0, %0  \n\t"
            :
            : "r"(frequency * front->trigger_time));
    }
    else
    {
        user_timer = 0;
        sys_disable_core_timer();
    }

    return;
}
