#include <coretimer.h>

#include <stdint.h>

#include <utils.h>
#include <uart.h>
#include <allocator.h>
#include <interrupt.h>

TIMER* timer_queue;

void coretimer_el0_enable(){
    asm("msr cntp_ctl_el0, %0"::"r"((uint64_t)1));
}

void timer_queue_push(TIMER* new_timer){
    interrupt_disable();
    if(!timer_queue){
        timer_queue = new_timer;
        goto ret;
    }
    TIMER* head = timer_queue;
    TIMER* prev = 0;
    if(head->time > new_timer->time){
        new_timer->next = head;
        timer_queue = new_timer;
        goto ret;
    }
    while(head->next){
        prev = head;
        head = head->next;
        if(head->time > new_timer->time){
            new_timer->next = head;
            prev->next = new_timer;
        goto ret;
        }
    }
    head->next = new_timer;
ret:
    interrupt_enable();
}

TIMER* timer_queue_pop(){
    if(!timer_queue) return 0;
    interrupt_disable();
    TIMER* timer = timer_queue;
    timer_queue = timer_queue->next;
    interrupt_enable();
    return timer;
}

TIMER* timer_queue_top(){
    if(!timer_queue) return 0;
    return timer_queue;
}

void timer_sched(){
    TIMER* timer = timer_queue_top();
    if(!timer) return ;
    uint64_t cntpct_el0;
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    asm("msr cntp_tval_el0, %0"::"r"((uint32_t)timer->time-cntpct_el0));
    memory_write(CORE0_TIMER_IRQ_CTRL, 2);
}

void add_timer(uint64_t time_wait, void (*func)(void *), void *arg){
    uint32_t cntfrq_el0;
    uint64_t cntpct_el0;
    asm("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    
    uint64_t time = cntpct_el0 + time_wait * cntfrq_el0;

    TIMER* new_timer = (TIMER*)simple_malloc(sizeof(TIMER));
    new_timer->time = time;
    new_timer->func = func;
    new_timer->arg = arg;
    new_timer->next = 0;

    timer_queue_push(new_timer);
    timer_sched();
}

void coretimer_el0_handler(){
    memory_write(CORE0_TIMER_IRQ_CTRL, 0);
    uint64_t cntpct_el0;
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));

    TIMER* timer;
    while(1){
        interrupt_disable();
        timer = timer_queue_top();
        if(!timer){
            interrupt_enable();
            break;
        }
        if(timer->time > cntpct_el0) break;
        timer_queue_pop();
        interrupt_enable();
        timer->func(timer->arg);
    }
    timer_sched();
}