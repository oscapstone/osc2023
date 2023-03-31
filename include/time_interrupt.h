#ifndef _TIME_INTERRUPT_H
#define _TIME_INTERRUPT_H
#include "utils.h"
#include "mmio.h"
//https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf p7
//https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf 4.6
#define CORE0_TIMER_IRQ_CTRL ((volatile unsigned int *)(0x40000040))
//https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf 4.10
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(0x40000060))
//https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf 7.5
#define ARM_IRQ_REG_BASE ((volatile unsigned int*)(MMIO_BASE + 0x0000b000))
#define IRQ_PENDING_1 	 ((volatile unsigned int*)(MMIO_BASE + 0x0000b204))
#define ENB_IRQS1 		 ((volatile unsigned int*)(MMIO_BASE + 0x0000b210))
#define DISABLE_IRQS1 	 ((volatile unsigned int*)(MMIO_BASE + 0x0000b21c))
#define get_timer_ticks() read_sysreg(cntpct_el0)
#define get_timer_freq() read_sysreg(cntfrq_el0)
#define get_timer_interval(start, end) (((end) - (start)) / get_timer_freq())
#define timer_on() write_sysreg(cntp_ctl_el0, 1)
#define timer_off() write_sysreg(cntp_ctl_el0, 0)
#define set_timer_cntp_cval_el0(t) write_sysreg(cntp_cval_el0, (t))
#define set_timer_cntp_tval_el0(tval) write_sysreg(cntp_tval_el0, (tval))
extern void write_uptime();
typedef void (*timer_interrupt_callback_t) (void *data);
typedef struct timer_task_node {
    //some other member variable
    unsigned long long run_at;
    timer_interrupt_callback_t callback;
    void *data;
    struct timer_task_node *next, *prev;
} timer_task_t;
struct timer_task_scheduler {
    //member variable
    //  doubled-linked-list queue head sorted by run_at in increasing order
    timer_task_t *head;
    //  size
    size_t qsize;

    //member function
    //  init
    // void (*init) (struct timer_task_scheduler *self);
    //  insert
    int (*insert) (struct timer_task_scheduler *self, timer_task_t *task);
    //  unlink
    int (*unlink) (struct timer_task_scheduler *self, timer_task_t *task);
    //  unlink head
    int (*unlink_head) (struct timer_task_scheduler *self);
    //  handler
    void (*timer_interrupt_handler) (struct timer_task_scheduler *self);
    //  add_timer_second
    //  @param1: callback
    //  @param2: data
    //  @param3: duration in seconds
    //  return 1 if timer was added successfully, 0 if timer has expired or has problem on inserting to queue
    int (*add_timer_second) (struct timer_task_scheduler *self, timer_interrupt_callback_t callback, void *data, size_t duration);
    void (*interval_run_second)(struct timer_task_scheduler *self, timer_interrupt_callback_t callback, void *data, size_t duration);
};
extern void timer_task_scheduler_init(struct timer_task_scheduler *self);
extern struct timer_task_scheduler _timer_task_scheduler;
extern void print_uptime_every2second();
extern void notify(void *arg);
extern void sleep_timer(void *arg);
#endif