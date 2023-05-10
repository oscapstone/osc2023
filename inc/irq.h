#ifndef _IRQ_H
#define _IRQ_H

#include <type.h>
#include <list.h>
#define TIMER_PRIO 0
#define UART_PRIO  1

typedef struct{
    void (*callback)(void *);
    void *data;
    uint32 priority;
    int running;
    struct list_head dp;
    struct list_head sp;
} irq_node;

typedef struct{
    struct list_head dp_head;
    uint32 i_status;
    uint32 i_nested_layer;
} irq_meta;

int irq_add_task(void (*task)(void *), void *data, void (*fini)(void), uint32 priority);
void irq_init();
void default_exception_handler(uint32 n);
void irq_handler();
void enable_irqs1();


#endif