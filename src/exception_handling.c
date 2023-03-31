#include "utils.h"
#include "uart.h"
#include "mini_uart.h"
#include "exception.h"
#include "time_interrupt.h"
#include "salloc.h"
struct interrupt_scheduler _interrupt_scheduler;
//later lab: https://oscapstone.github.io/labs/lab3.html#exception-handling

void default_handler()
{
    disable_local_all_interrupt();

    unsigned long spsr = read_sysreg(spsr_el1);
    unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    uart_write_string("spsr_el1: ");
    uart_write_no_hex(spsr);
    uart_write_string("\n");

    uart_write_string("elr_el1: ");
    uart_write_no_hex(elr);
    uart_write_string("\n");

    uart_write_string("esr_el1: ");
    uart_write_no_hex(esr);
    uart_write_string("\n");
    
    enable_local_all_interrupt();
}

void current_synchronous_exception_router(void)
{
    uart_write_string("In current_synchronous_exception_router\n");
    default_handler();
}

void _qbin_unlink_head(struct interrupt_scheduler *self, struct interrupt_task_node **tail)
{
    if (*tail == (*tail)->next) {
        *tail = NULL;
    } else {
        (*tail)->next = ((*tail)->next->next);
    }
    self->qsize--;
}

struct interrupt_task_node *fetch_next(struct interrupt_scheduler *self, int *prior) {
    if (self->qsize == 0) return NULL;
    struct interrupt_task_node *task = NULL;
    for (int i = PRIORITY_BINS - 1; i >= 0; i--) {
        if (self->qbins_tail[i] != NULL) {
            task = self->qbins_tail[i]->next; //qbin head
            *prior = i;
            _qbin_unlink_head(self, self->qbins_tail + i);
            break;
        }
    }
    return task;
}

//Decouple the Interrupt Handlers
void current_irq_exception_router(void)
{
    disable_local_all_interrupt();
    int new_prior = -1;
    // uart_write_string("In current_irq_exception_router\n");
    //top half
    ////https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p112
    unsigned int uart = (*IRQ_PENDING_1 & (1 << 29));
    //https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf 4.10
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if (uart) {
        //handle uart (Basic Exercise 3)
        //https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p12
        //1. masks the device’s interrupt line,
        //disable uart interrupt
        if (*AUX_MU_IER & 0b10) *AUX_MU_IER &= ~(0b10);
        else if (*AUX_MU_IER & 0b01) *AUX_MU_IER &= ~(0b01);
        // *AUX_MU_IER &= ~(0b11);
        //3. enqueues the processing task to the event queue,
        _interrupt_scheduler.add_task(&_interrupt_scheduler, uart_irq_handler, *AUX_MU_IER, 1);
        new_prior = 1;
    } else if (core_timer) {
        //handle timer interrupt
        //Basic Exercise 2 - Interrupt
        /*
        Todo
            Enable the core timer’s interrupt. 
            The interrupt handler should print 
            the seconds after booting and set 
            the next timeout to 2 seconds later.
        */
        //1. masks the device’s interrupt line,
        core_timer_disable();
        //3. enqueues the processing task to the event queue,
        _interrupt_scheduler.add_task(&_interrupt_scheduler, _timer_task_scheduler.timer_interrupt_handler, &_timer_task_scheduler, 0);
        new_prior = 0;
    }

    // uart_write_string("exception scheduler qsize: ");
    // uart_write_no(_interrupt_scheduler.qsize);
    // uart_write_string("\n");
    
    //unmasks the interrupt line to get the next interrupt at the end of the task.
    //Preemption
    //the newly enqueued task still needs to wait for the currently running task’s completion.
    //Notice that the newly incoming task can still be inserted in this function call. (see code above)
    //bottom half
    disable_local_all_interrupt();
    int prior;
    
    // if (_interrupt_scheduler.prior_stack == NULL || new_prior > _interrupt_scheduler.prior_stack->priority) {    
    if (_interrupt_scheduler.prior_stk_idx != -1 && new_prior > _interrupt_scheduler.priority_stack[_interrupt_scheduler.prior_stk_idx]) {
        //Preemption
        struct interrupt_task_node *task = fetch_next(&_interrupt_scheduler, &prior);
        // insert_prior(&_interrupt_scheduler, prior);
        _interrupt_scheduler.priority_stack[++(_interrupt_scheduler.prior_stk_idx)] = prior;
        //4. do the tasks with interrupts enabled,
        enable_local_all_interrupt();
        //run single task
        task->handler(task->data);
        disable_local_all_interrupt();
        // pop_prior(&_interrupt_scheduler);
        _interrupt_scheduler.prior_stk_idx--;
    // } else if (_interrupt_scheduler.prior_stack == NULL) {
    } else if (_interrupt_scheduler.prior_stk_idx == -1) {
        //is not running
        while (_interrupt_scheduler.qsize) {
            struct interrupt_task_node *task = fetch_next(&_interrupt_scheduler, &prior);
            // insert_prior(&_interrupt_scheduler, prior);
            _interrupt_scheduler.priority_stack[++(_interrupt_scheduler.prior_stk_idx)] = prior;
            //4. do the tasks with interrupts enabled,
            enable_local_all_interrupt();
            //run single task
            task->handler(task->data);
            disable_local_all_interrupt();
            // pop_prior(&_interrupt_scheduler);
            _interrupt_scheduler.prior_stk_idx--;
        }
    } //else {
        //other one is running, that guy will wake handle the newly enqueued task
    //}
    enable_local_all_interrupt();
}
void lower_synchronous_exception_router(void)
{
    uart_write_string("In lower_synchronous_exception_router\n");
    default_handler();
}
void lower_irq_exception_router(void)
{
    uart_write_string("In lower_irq_exception_router\n");
    default_handler();
}

void _insert_qbin(struct interrupt_scheduler *self, struct interrupt_task_node **tail, struct interrupt_task_node *task)
{
    if (*tail == NULL) {
        task->next = task;
        *tail = task;
    } else {
        task->next = (*tail)->next;
        (*tail)->next = task;
        *tail = task;
    }
    self->qsize++;
}

void _add_task(struct interrupt_scheduler *self, interrupt_handler_t handler, void *data, unsigned priority)
{
    //avoid re-entrance
    disable_local_all_interrupt();
    struct interrupt_task_node *new_node = (struct interrupt_task_node *)simple_malloc(sizeof(struct interrupt_task_node));
    new_node->handler = handler;
    new_node->data = data;
    _insert_qbin(self, self->qbins_tail + priority, new_node);
    enable_local_all_interrupt();
}

void init_interrupt_scheduler(struct interrupt_scheduler *self)
{
    // self->prior_stack = NULL;
    self->prior_stk_idx = -1;
    for (int i = 0; i < PRIORITY_BINS; i++) {
        self->qbins_tail[i] = NULL;
        self->priority_stack[i] = NULL;
    }
    self->qsize = 0;
    self->add_task = _add_task;
}