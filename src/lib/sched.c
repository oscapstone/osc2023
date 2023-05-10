#include <sched.h>
#include <type.h>
#include <current.h>
#include <timer.h>
#include <list.h>
#include <preempt.h>

#define SCHED_TIMER_FREQ 32
#define SCHED_WATERMARK 1

static struct list_head run_queue;
static uint32 sched_ticks;

static void timer_sched_tick(void *_){
    sched_tick();

    timer_add_freq(timer_sched_tick, NULL, SCHED_TIMER_FREQ);
}

void scheduler_init(void)
{
    INIT_LIST_HEAD(&run_queue);

    timer_add_freq(timer_sched_tick, NULL, SCHED_TIMER_FREQ);
}

void sched_tick(){
    sched_ticks ++;
    if(sched_ticks >= SCHED_WATERMARK){
        sched_ticks = 0;
        current->need_resched = 1;
    }
}

void sched_add_task(task_struct *task){
    uint32 daif;
    daif = save_and_disable_interrupt();
    list_add_tail(&task->list, &run_queue);
    restore_interrupt(daif);
}

void schedule(void){
    uint64 daif;
    task_struct *task;
    daif = save_and_disable_interrupt();
    task = list_first_entry(&run_queue, task_struct, list);

    list_del(&task->list);
    list_add_tail(&task->list, &run_queue);

    current->need_resched = 0;
    restore_interrupt(daif);
    switch_to(current, task);
}

void sched_del_task(task_struct *task){
    preempt_disable();

    list_del(&task->list);

    preempt_enable();
}