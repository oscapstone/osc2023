#include "exception.h"
#include "uart.h"
#include "peripherals/irq.h"
#include "peripherals/mini_uart.h"
#include "timer.h"
#include "malloc.h"

// DAIF, Interrupt Mask Bits
void el1_interrupt_enable(){
    __asm__ __volatile__("msr daifclr, 0xf"); // umask all DAIF to 1111
}

void el1_interrupt_disable(){
    __asm__ __volatile__("msr daifset, 0xf"); // mask all DAIF
}

void invalid_exception_router(unsigned long long x0){
    //uart_sendline("invalid execption: 0x%x\r\n",x0);
}

void el0_sync_router(){
    uart_send_string("in el0 sync");
    unsigned long long spsr_el1;
    __asm__ __volatile__("mrs %0, SPSR_EL1\n" : "=r" (spsr_el1)); // EL1 configuration, spsr_el1[9:6]=4b0 to enable interrupt
    unsigned long long elr_el1;
    __asm__ __volatile__("mrs %0, ELR_EL1\n" : "=r" (elr_el1));   // ELR_EL1: return address if return to EL1
    unsigned long long esr_el1;
    __asm__ __volatile__("mrs %0, ESR_EL1\n" : "=r" (esr_el1));   // ESR_EL1:symdrome information
    uart_send_string("[Exception][el0_sync] spsr_el1 :0x");
    uart_hex(spsr_el1);
    uart_send_string(" ,elr_el1 :0x");
    uart_hex(elr_el1);
    uart_send_string(" ,esr_el1 :0x");
    uart_hex(esr_el1);
    uart_send_string("\n");
}

void el0_irq_64_router(){
    uart_sendline("in el0 irq \n");
    // decouple the handler into irqtask queue
    // (1) https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf - Pg.113
    // (2) https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf - Pg.16
    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception
    {
        if (*AUX_MU_IIR & (0b01 << 1))
        {
            *AUX_MU_IER &= ~(2);  // disable write interrupt
            irqtask_add(uart_w_irq_handler, UART_IRQ_PRIORITY);
            irqtask_run_preemptive();
        }
        else if (*AUX_MU_IIR & (0b10 << 1))
        {
            *AUX_MU_IER &= ~(1);  // disable read interrupt
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



void el1h_irq_router(){
    // decouple the handler into irqtask queue
    // (1) https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf - Pg.113
    // (2) https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf - Pg.16
    if(*IRQ_PENDING_1 & IRQ_PENDING_1_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU) // from aux && from GPU0 -> uart exception
    {
        //uart_sendline("here");
        if (*AUX_MU_IIR & (1 << 1))
        {
            *AUX_MU_IER &= ~(2);  // disable write interrupt
            irqtask_add(uart_w_irq_handler, UART_IRQ_PRIORITY);
            irqtask_run_preemptive(); // run the queued task before returning to the program.
        }
        else if (*AUX_MU_IIR & (2 << 1))
        {
            *AUX_MU_IER &= ~(1);  // disable read interrupt
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





/* implement advance 2 */
int curr_task_priority = 9999;   // Small number has higher priority

struct list_head *task_list;
void irqtask_list_init()
{
    INIT_LIST_HEAD(task_list);
}


void irqtask_add(void *task_function,unsigned long long priority){
    irqtask_t *the_task = malloc(sizeof(irqtask_t)); // free by irq_tasl_run_preemptive()

    // store all the related information into irqtask node
    // manually copy the device's buffer
    the_task->priority = priority;
    the_task->task_function = task_function;
    INIT_LIST_HEAD(&the_task->listhead);

    // add the timer_event into timer_event_list (sorted)
    // if the priorities are the same -> FIFO
    struct list_head *curr;

    // mask the device's interrupt line
    el1_interrupt_disable();
    // enqueue the processing task to the event queue with sorting.
    list_for_each(curr, task_list)
    {
        if (((irqtask_t *)curr)->priority > the_task->priority)
        {
            list_add(&the_task->listhead, curr->prev);
            break;
        }
    }
    // if the priority is lowest
    if (list_is_head(curr, task_list))
    {
        list_add_tail(&the_task->listhead, task_list);
    }
    // unmask the interrupt line
    el1_interrupt_enable();
}

void irqtask_run_preemptive(){
    el1_interrupt_enable();
    while (!list_empty(task_list))
    {
        // critical section protects new coming node
        el1_interrupt_disable();
        irqtask_t *the_task = (irqtask_t *)task_list->next;
        // Run new task (early return) if its priority is lower than the scheduled task.
        if (curr_task_priority <= the_task->priority)
        {
            el1_interrupt_enable();
            break;
        }
        // get the scheduled task and run it.
        list_del_entry((struct list_head *)the_task);
        int prev_task_priority = curr_task_priority;
        curr_task_priority = the_task->priority;

        el1_interrupt_enable();
        irqtask_run(the_task);
        el1_interrupt_disable();

        curr_task_priority = prev_task_priority;
        el1_interrupt_enable();
        free(the_task);
    }
}

void irqtask_run(irqtask_t* the_task)
{
    ((void (*)())the_task->task_function)();
}


