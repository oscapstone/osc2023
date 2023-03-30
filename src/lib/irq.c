#include <irq.h>
#include <mini_uart.h>
#include <utils.h>
#include <timer.h>
#include <BCM.h>

#define IRQ_TASK_NUM 32

irq_node irq_nodes[IRQ_TASK_NUM];
irq_meta i_meta;

static irq_node *irq_alloc(){
    uint32 idx = __builtin_ffs(i_meta.i_status)-1;
    if(idx<0)
        return NULL;
    
    i_meta.i_status &= ~(1<<idx);
    return &irq_nodes[idx];
} 

static void irq_free(irq_node *irqn){
    if(!irqn)
        return;
    uint32 idx = get_elem_idx(irqn, irq_nodes);
    i_meta.i_status |= (1<<idx);
}

static int irq_insert(irq_node *irqn){

    irq_node *entry;
    int preempt = 1, insert = 0;
    list_for_each_entry(entry,&i_meta.dp_head,dp){
        if(irqn->priority > entry->priority){
            list_add_tail(&irqn->dp,&entry->dp);
            insert = 1;
            break;
        }
        else if(irqn->priority == entry->priority){
            list_add_tail(&irqn->sp,&entry->sp);
            preempt = 0;
            insert = 1;
            break;
        }
        preempt = 0;
    }
    if(!insert)
        list_add_tail(&irqn->dp,&i_meta.dp_head);
    
    return preempt;
}

static void irq_remove(irq_node *irqn){
    if(!list_empty(&irqn->dp)){
        if(list_empty(&irqn->sp))
            list_del(&irqn->dp);
        else{
            irq_node *left = list_last_entry(&irqn->dp, irq_node, dp);
            irq_node *down = list_first_entry(&irqn->sp, irq_node, sp);
            list_del(&irqn->dp);
            list_add(&down->dp,&left->dp);
            list_del(&irqn->sp);
        }
    }
    else
        list_del(&irqn->sp);
    irq_free(irqn);
}

static void irq_run(irq_node *irqn){
    if(!irqn)
        return;
    
    irqn->running = 1;
    // critical section
    enable_interrupt();
    (irqn->callback)(irqn->data);
    disable_interrupt();
    irq_remove(irqn);
}

static irq_node *get_next_task(){
    if(list_empty(&i_meta.dp_head))
        return NULL;
    
    irq_node *first = list_first_entry(&i_meta.dp_head, irq_node, dp);
    if(first->running)
        return NULL;

    return first;
}

static void irq_loop(){
    while(1){
        irq_node *cur = get_next_task();
        if(!cur)
            break;
        irq_run(cur);
    }
}

int add_task(void (*callback)(void *), void *data, uint32 priority){
    irq_node *new_irqn = irq_alloc();
    if(!new_irqn)
        return -1;
    new_irqn->callback = callback;
    new_irqn->data = data;
    new_irqn->priority = priority;
    new_irqn->running = 0;
    INIT_LIST_HEAD(&new_irqn->dp);
    INIT_LIST_HEAD(&new_irqn->sp);

    int preempt = irq_insert(new_irqn);
    if(preempt)
        irq_run(new_irqn);
    irq_loop();
    return 0;
}

void irq_init(){
    i_meta.i_status = 0xffffffff;
    INIT_LIST_HEAD(&i_meta.dp_head);
}

void default_exception_handler(uint32 n){
    uart_printf("[exception] %d\r\n", n);
}

void irq_handler(){
    timer_irq_add();
    uart_irq_add();
}

void enable_irqs1(){
    put32(ENABLE_IRQS1, 1 << 29);             // Enable UART1 IRQ
}