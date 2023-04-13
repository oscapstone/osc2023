#include "timer.h"
#include "uart.h"
#include "malloc.h"
#include "string.h"

list_head_t timer_event_list;

void timer_list_init()
{
    INIT_LIST_HEAD(&timer_event_list);
}

void core_timer_enable()
{
    asm volatile(
        "mov x1, 1\n\t"
        "msr cntp_ctl_el0, x1\n\t");
}

void core_timer_disable()
{
    asm volatile(
        "mov x1, 0\n\t"
        "msr cntp_ctl_el0, x1\n\t");
}

void core_timer_interrupt_enable()
{
    asm volatile(
        "mov x2, 2\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t");
}

void core_timer_interrupt_disable()
{
    asm volatile(
        "mov x2, 0\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t");
}

void core_timer_handler()
{
    if (list_empty(&timer_event_list))
    {
        //core_timer_interrupt_disable(); // Buggy
        core_timer_interrupt_disable_alternative();
        return;
    }

    pop_timer();
}

void add_timer(timer_callback_t callback, char *args, unsigned long long timeout)
{
    timer_event_t *the_timer_event = smalloc(sizeof(timer_event_t));

    the_timer_event->args = smalloc(strlen(args) + 1);
    strcpy(the_timer_event->args, args);

    the_timer_event->expire_time = get_expired_tick_relative(timeout);
    the_timer_event->callback = callback;
    INIT_LIST_HEAD(&the_timer_event->listhead);

    list_head_t *curr;

    list_for_each(curr, &timer_event_list)
    {
        if (((timer_event_t *)curr)->expire_time > the_timer_event->expire_time)
        {
            list_add(&the_timer_event->listhead, curr->prev);
            break;
        }
    }

    if (list_is_head(curr, &timer_event_list))
        list_add_tail(&the_timer_event->listhead, &timer_event_list);

    set_core_timer_interrupt_by_tick(((timer_event_t *)timer_event_list.next)->expire_time);
    // core_timer_enable();
}

void pop_timer()
{
    timer_event_t *cur = (timer_event_t *)timer_event_list.next;
    list_del_entry((list_head_t *)cur);
    cur->callback(cur->args);

    if (list_empty(&timer_event_list))
        //core_timer_interrupt_disable(); // Buggy
        core_timer_interrupt_disable_alternative();
    else
        set_core_timer_interrupt_by_tick(((timer_event_t *)timer_event_list.next)->expire_time);
}

void two_second_alert(char *str)
{
    uart_printf("'%s': seconds after booting : %d\n", str, get_clock_time());
    add_timer(two_second_alert, str, 2);
}

void core_timer_interrupt_disable_alternative()
{
    set_core_timer_interrupt(1000);
}

/* Utility functions */

unsigned long long get_clock_tick()
{
    unsigned long long cntpct_el0;
    __asm__ __volatile__(
        "mrs %0, cntpct_el0\n\t"
        : "=r"(cntpct_el0)); // tick now

    return cntpct_el0;
}

unsigned long long get_clock_freq()
{
    unsigned long long cntfrq_el0;
    __asm__ __volatile__(
        "mrs %0, cntfrq_el0\n\t"
        : "=r"(cntfrq_el0)); // tick frequency
    return cntfrq_el0;
}

unsigned long long get_clock_time()
{
    return get_clock_tick() / get_clock_freq();
}

unsigned long long get_interrupt_tick()
{
    unsigned long long cntp_cval_el0;
    __asm__ __volatile__(
        "mrs %0, cntp_cval_el0\n\t"
        : "=r"(cntp_cval_el0));
    return cntp_cval_el0;
}

unsigned long long get_expired_tick_relative(unsigned long long second)
{
    return (get_clock_tick() + get_clock_freq() * second);
}

void set_core_timer_interrupt(unsigned long long expired_time)
{
    __asm__ __volatile__(
        "mrs x1, cntfrq_el0\n\t" //cntfrq_el0 -> relative time
        "mul x1, x1, %0\n\t"
        "msr cntp_tval_el0, x1\n\t" // set expired time
        :: "r"(expired_time):"x1");
}

void set_core_timer_interrupt_by_tick(unsigned long long tick)
{
    __asm__ __volatile__(
        "msr cntp_cval_el0, %0\n\t" // cntp_cval_el0 -> absolute time
        ::"r"(tick));
}


int timer_list_get_size()
{
    int r = 0;
    list_head_t *curr;
    list_for_each(curr, &timer_event_list)
    {
        r++;
    }
    return r;
}