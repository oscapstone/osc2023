#include "bcm2837/rpi_irq.h"
#include "bcm2837/rpi_uart1.h"
#include "uart1.h"
#include "exception.h"
#include "timer.h"
#include "heap.h"

void el1_interrupt_enable(){
    __asm__ __volatile__("msr daifclr, 0xf");
}

void el1_interrupt_disable(){
    __asm__ __volatile__("msr daifset, 0xf");
}

void el1h_irq_router(){
    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception
    {
        if (*AUX_MU_IIR_REG & (1 << 1))
        {
            *AUX_MU_IER_REG &= ~(2);  // disable write interrupt
            irqtask_add(uart_w_irq_handler, UART_IRQ_PRIORITY);
            irqtask_run_preemptive();
        }
        else if (*AUX_MU_IIR_REG & (2 << 1))
        {
            *AUX_MU_IER_REG &= ~(1);  // disable read interrupt
            irqtask_add(uart_r_irq_handler, UART_IRQ_PRIORITY);
            irqtask_run_preemptive();
        }
    }
    else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)  //from CNTPNS (core_timer) // A1 - setTimeout run in el1
    {
        core_timer_disable();
        irqtask_add(core_timer_handler, TIMER_IRQ_PRIORITY);
        irqtask_run_preemptive();
        core_timer_enable();
    }
}

void el0_sync_router(){
    unsigned long long spsr_el1;
    __asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r" (spsr_el1));
    unsigned long long elr_el1;
    __asm__ __volatile__("mrs %0, ELR_EL1\n\t" : "=r" (elr_el1));
    unsigned long long esr_el1;
    __asm__ __volatile__("mrs %0, ESR_EL1\n\t" : "=r" (esr_el1));
    uart_sendline("## Exception - el0_sync ## spsr_el1 : 0x%x, elr_el1 : 0x%x, esr_el1 : 0x%x\n", spsr_el1, elr_el1, esr_el1);
}

void el0_irq_64_router(){
    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception
    {
        if (*AUX_MU_IIR_REG & (0b01 << 1))
        {
            *AUX_MU_IER_REG &= ~(2);  // disable write interrupt
            irqtask_add(uart_w_irq_handler, UART_IRQ_PRIORITY);
            irqtask_run_preemptive();
        }
        else if (*AUX_MU_IIR_REG & (0b10 << 1))
        {
            *AUX_MU_IER_REG &= ~(1);  // disable read interrupt
            irqtask_add(uart_r_irq_handler, UART_IRQ_PRIORITY);
            irqtask_run_preemptive();
        }
    }
    else if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)  //from CNTPNS (core_timer) // A1 - setTimeout run in el1
    {
        core_timer_disable();
        irqtask_add(core_timer_handler, TIMER_IRQ_PRIORITY);
        irqtask_run_preemptive();
        core_timer_enable();
    }
}


void invalid_exception_router(unsigned long long x0){
    uart_sendline("invalid exception : 0x%x\r\n",x0);
    while(1);
}

// ------------------------------------------------------------------------------------------

/*
Preemption
Now, any interrupt handler can preempt the task’s execution, but the newly enqueued task still needs to wait for the currently running task’s completion. It’d be better if the newly enqueued task with a higher priority can preempt the currently running task.
To achieve the preemption, the kernel can check the last executing task’s priority before returning to the previous interrupt handler. If there are higher priority tasks, execute the highest priority task.
*/
int curr_task_priority = 9999;   // init a very low priority

struct list_head *task_list;

void irqtask_list_init()
{
    INIT_LIST_HEAD(task_list);
}

//like add_timer
void irqtask_add(void *task_function,unsigned long long priority){
    irqtask_t *the_task = kmalloc(sizeof(irqtask_t)); //need to free by task runner

    the_task->priority = priority; // store interrupt time into timer_event
    the_task->task_function = task_function;
    INIT_LIST_HEAD(&the_task->listhead);

    // add the timer_event into timer_event_list (sorted) 
    // if the same priority FIFO
    struct list_head *curr;

    el1_interrupt_disable(); // critical section
    list_for_each(curr, task_list)
    {
        if (((irqtask_t *)curr)->priority > the_task->priority)
        {
            list_add(&the_task->listhead, curr->prev); // add this timer at the place just before the bigger one (sorted)
            break;
        }
    }

    if (list_is_head(curr, task_list))
    {
        list_add_tail(&the_task->listhead, task_list); // for the time is the biggest
    }
    el1_interrupt_enable();
}

void irqtask_run_preemptive(){
    el1_interrupt_enable(); //do the tasks with interrupts enabled, (lab3 advanced 2 )
    while (!list_empty(task_list))
    {
        el1_interrupt_disable();  // critical section
        irqtask_t *the_task = (irqtask_t *)task_list->next;
        // just run task when priority is lower than the task preempted
        if (curr_task_priority <= the_task->priority)
        {
            el1_interrupt_enable();
            break; //
        }
        list_del_entry((struct list_head *)the_task);
        int prev_task_priority = curr_task_priority;
        curr_task_priority = the_task->priority;
        el1_interrupt_enable();

        irqtask_run(the_task);

        el1_interrupt_disable(); // critical section
        curr_task_priority = prev_task_priority;
        el1_interrupt_enable();
        free(the_task);
    }
}

void irqtask_run(irqtask_t* the_task)
{
    ((void (*)())the_task->task_function)();
}

