#ifndef _TIMER_H
#define _TIMER_H

#include <type.h>
#include <list.h>

typedef struct {
    void (*callback)(void *);
    void *data;
    uint32 time_left;
    struct list_head lh;
} timer_node;

/* t_status has 32 bits, n-th bit stands for the status of t_procs[n].
 * If n-th bit is 1, it means t_procs[n] is available.
 * If n-th bit is 0, it means t_procs[n] is unavailable.
 */

 /* When setting a timer, t_interval will store the interval.
 * It is 0 when timer is not set.
 */
typedef struct{
    struct list_head lh;
    uint32 t_status;
    uint32 t_interval;
    int size;
} timer_meta;

void timer_init();
void boot_time_callback();
void timer_irq_add();
void timer_irq_handler();
void timer_switch_info();
void timer_add_after(void (*callback)(void *), void *data, uint32 after);
void timer_add_freq(void (*callback)(void *), void *data, uint32 freq);

#endif