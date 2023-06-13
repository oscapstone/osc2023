#include "time_interrupt.h"
#include "mini_uart.h"
#include "mm.h"
#include "uart.h"
#include "exception.h"
#include "utils.h"
struct timer_task_scheduler _timer_task_scheduler;
extern void core_timer_enable();
extern void core_timer_disable();
extern void timer_enable_int();
extern void timer_disable_int();
void write_uptime()
{
    uart_write_string("Booting time: ");
    unsigned long long cur_ticks = get_timer_ticks();
    unsigned long long freq = get_timer_freq();
    uart_write_fraction(cur_ticks, freq, 5);
    uart_write_string(" seconds\n");
}

//should be protected in critical section
int _insert(struct timer_task_scheduler *self, timer_task_t *task)
{
    //https://wzhchen.github.io/uboot/uboot%E4%B8%8B%E5%BC%80%E5%90%AFARMv8%E7%9A%84%E5%AE%9A%E6%97%B6%E5%99%A8%E4%B8%AD%E6%96%AD/
    timer_off();
    unsigned long long cur_ticks = get_timer_ticks();
    if (task->run_at <= cur_ticks) {
        //task has expired.
        timer_on();
        return 0;
    }
    //precedence: '->' > '&'
    INIT_LIST_HEAD(&task->node);
    //last node which run_at < that of current task.
    list_t *insert_pos = &self->head, *node;
    list_for_each(node, &self->head) {
        if (list_entry(node, timer_task_t, node)->run_at < task->run_at) {
            insert_pos = node;
        }
        break;
    }
    list_add_tail(&task->node, insert_pos);
    self->qsize++;

    if (self->head.next == &task->node) {
        //the newly inserted task is the first task to be executed
        //update expire time
        set_timer_cntp_cval_el0(task->run_at);
    }
    timer_on();
    return 1;
}

//should be protected in critical section
int _unlink(struct timer_task_scheduler *self, timer_task_t *task)
{
    list_del(&task->node);
    self->qsize--;
    //ask timer to intterupt on the next event
    if (!list_empty(&self->head)) {
        core_timer_enable();
        timer_task_t *first_task = list_entry(self->head.next, timer_task_t, node);
        set_timer_cntp_cval_el0(first_task->run_at);
    }
    return 1;
}

int _unlink_head(struct timer_task_scheduler *self)
{
    if (list_empty(&self->head)) return 0;
    return self->unlink(self, list_entry(self->head.next, timer_task_t, node));
}

//run those tasks have expired in queue and pop them.
void _timer_interrupt_handler(struct timer_task_scheduler *self)
{
    task_t *current = get_current_thread();
    unsigned long long cur_ticks = get_timer_ticks();
    while (!list_empty(&self->head)) {
        timer_task_t *first_task = list_entry(self->head.next, timer_task_t, node);
        if (first_task->run_at > cur_ticks) break;
        self->unlink_head(self);
        // data crash?
        // create_thread_with_argc_argv(first_task->callback, 1, &first_task->data);
        first_task->callback(first_task->data);
        kfree(first_task);
        cur_ticks = get_timer_ticks();
    }
}



timer_task_t *new_task(timer_interrupt_callback_t callback, void *data, size_t duration)//in ticks
{
    timer_task_t *task = (timer_task_t *)kmalloc(sizeof(timer_task_t));
    task->callback = callback;
    task->data = data;
    unsigned long long cur_ticks = get_timer_ticks();
    task->run_at = cur_ticks + duration;
    return task;
}
/*
add_timer_second
    @param1: callback
    @param2: data
    @param3: duration in seconds
    return 1 if timer was added successfully, 0 if timer has expired or has problem on inserting to queue
*/
int _add_timer_second(struct timer_task_scheduler *self, timer_interrupt_callback_t callback, void *data, size_t duration)
{
    unsigned long long freq = get_timer_freq();
    timer_task_t *task = new_task(callback, data, duration * freq);
    return self->insert(self, task);
}

/*
add_timer_tick
    @param1: callback
    @param2: data
    @param3: duration in tick
    return 1 if timer was added successfully, 0 if timer has expired or has problem on inserting to queue
*/
int _add_timer_tick(struct timer_task_scheduler *self, timer_interrupt_callback_t callback, void *data, size_t ticks)
{
    timer_task_t *task = new_task(callback, data, ticks);
    return self->insert(self, task);
}

struct interval_data {
    struct timer_task_scheduler *scheduler;
    timer_interrupt_callback_t callback;
    void *data;
    size_t duration;//in ticks
};
struct interval_data *new_interval_data(struct timer_task_scheduler *self, timer_interrupt_callback_t callback, void *data, size_t duration)
{
    struct interval_data *real_routine = (struct interval_data *)kmalloc(sizeof(struct interval_data));
    real_routine->callback = callback;
    real_routine->data = data;
    real_routine->scheduler = self;
    real_routine->duration = duration;//in ticks
}

void timer_interval_callback(void *data)
{
    struct interval_data *real_routine = (struct interval_data *)data;
    real_routine->callback(real_routine->data);
    // create_thread_with_argc_argv(real_routine->callback, 1, &real_routine->data);
    real_routine->scheduler->add_timer_tick(real_routine->scheduler, timer_interval_callback, real_routine, real_routine->duration);
}

void _interval_run_tick(struct timer_task_scheduler *self, timer_interrupt_callback_t callback, void *data, size_t duration)
{
    struct interval_data *real_routine = new_interval_data(self, callback, data, duration);
    self->add_timer_tick(self, timer_interval_callback, real_routine, duration);
}

void _interval_run_second(struct timer_task_scheduler *self, timer_interrupt_callback_t callback, void *data, size_t duration)
{
    unsigned long long freq = get_timer_freq();
    struct interval_data *real_routine = new_interval_data(self, callback, data, duration * freq);
    self->add_timer_second(self, timer_interval_callback, real_routine, duration);
}

void dbg_print_uptime()
{
    uart_write_string("cur_ticks: ");
    unsigned long long cur_ticks = get_timer_ticks();
    uart_write_no(cur_ticks);
    uart_write_string("\n");
    write_uptime();
}


//Enable the core timer’s interrupt. 
//The interrupt handler should print the seconds after booting and set the next timeout to 2 seconds later.
void print_uptime_every2second()
{
    _timer_task_scheduler.interval_run_second(&_timer_task_scheduler, (timer_interrupt_callback_t)write_uptime, NULL, 2);
    // _timer_task_scheduler.interval_run_second(&_timer_task_scheduler, (timer_interrupt_callback_t)dbg_print_uptime, NULL, 2);
}

void timer_task_scheduler_init(struct timer_task_scheduler *self)
{
    INIT_LIST_HEAD(&(self->head));
    self->qsize = 0;

    self->insert = _insert;
    self->unlink = _unlink;
    self->unlink_head = _unlink_head;
    self->timer_interrupt_handler = _timer_interrupt_handler;
    self->add_timer_second = _add_timer_second;
    self->interval_run_second = _interval_run_second;
    self->add_timer_tick = _add_timer_tick;
    self->interval_run_tick = _interval_run_tick;
}


unsigned long long print_cur_ticks()
{
    uart_write_string("cur_ticks: ");
    unsigned long long cur_ticks = get_timer_ticks();
    uart_write_no(cur_ticks);
    uart_write_string("\n");
    return cur_ticks;
}

void notify(void *arg)
{
    uart_write_string((char *)arg);
    //arg points to a chunk allocated by kmalloc
    kfree(arg);
}

void sleep_timer(void *arg)
{
    uart_write_string("in sleep_timer\n");
    // size_t t = (size_t)arg;
    unsigned long long freq = get_timer_freq();
    *ENB_IRQS1 |= (1 << 29);
    async_uart_write_string("asyncw\n");
    delay(freq);
    *DISABLE_IRQS1 &= ~(1 << 29);
    
    // delay(freq * t);
    uart_write_string("sleep_timer end\n");
}

void enable_el0_access_pcnter()
{
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
}

