#include "utils.h"
#include "uart.h"
#include "mini_uart.h"
#include "exception.h"
#include "time_interrupt.h"
#include "mm.h"
#include "stdint.h"
#include "syscall.h"
struct interrupt_scheduler _interrupt_scheduler;
//later lab: https://oscapstone.github.io/labs/lab3.html#exception-handling
int interrupt_cnter;
void test_enable_interrupt()
{
    if (interrupt_cnter > 0) interrupt_cnter--;
    if (interrupt_cnter == 0) {
        enable_local_all_interrupt();
    }
}
void disable_interrupt()
{
    disable_local_all_interrupt();
    interrupt_cnter++;
}

void default_handler()
{
    disable_interrupt();

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
    
    test_enable_interrupt();
}



void _qbin_unlink_head(struct interrupt_scheduler *self, list_t *head)
{
    list_del(head->next);
    self->qsize--;
}

struct interrupt_task_node *fetch_next(struct interrupt_scheduler *self, int *prior) {
    if (self->qsize == 0) return NULL;
    struct interrupt_task_node *task = NULL;
    for (int i = PRIORITY_BINS - 1; i >= 0; i--) {
        if (!list_empty(self->qbins + i)) {
            task = list_entry((self->qbins + i)->next, struct interrupt_task_node, list);
            *prior = i;
            _qbin_unlink_head(self, self->qbins + i);
            break;
        }
    }
    return task;
}

void core_timer_top_half(int prior)
{
    // uart_write_string("is timer\n");
    core_timer_disable();
    _interrupt_scheduler.add_task(&_interrupt_scheduler, _timer_task_scheduler.timer_interrupt_handler, &_timer_task_scheduler, prior);
    core_timer_enable();
}

void uart_top_half(int prior)
{
    // uart_write_string("is uart\n");
    //disable uart interrupt
    if (*AUX_MU_IER & 0b10) *AUX_MU_IER &= ~(0b10);
    else if (*AUX_MU_IER & 0b01) *AUX_MU_IER &= ~(0b01);
    // *AUX_MU_IER &= ~(0b11);
    _interrupt_scheduler.add_task(&_interrupt_scheduler, uart_irq_handler, *AUX_MU_IER, prior);
}

//Decouple the Interrupt Handlers
void irq_router(void)
{
    //https://grasslab.github.io/osdi/en/labs/lab3.html#on-excpetion-taken
    //Interrupt is disabled. (PSTATE.{D,A,I,F} are set to 1).
    //to enable kernel preemption, enable interrupt
    // if (interrupt_cnter == 0) enable_local_all_interrupt();
    
    disable_interrupt();
    int new_prior = -1;
    //top half
    unsigned int uart = (*IRQ_PENDING_1 & (1 << 29));
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    
    if (uart) {
        uart_top_half(1);
        new_prior = 1;
    } else if (core_timer) {
        core_timer_top_half(0);
        new_prior = 0;
    }

    //bottom half
    int prior;
    if (_interrupt_scheduler.prior_stk_idx != -1 && new_prior > _interrupt_scheduler.priority_stack[_interrupt_scheduler.prior_stk_idx]) {
        //Preemption
        struct interrupt_task_node *task = fetch_next(&_interrupt_scheduler, &prior);
        _interrupt_scheduler.priority_stack[++(_interrupt_scheduler.prior_stk_idx)] = prior;
        test_enable_interrupt();
        //run single task
        task->handler(task->data);
        disable_interrupt();
        _interrupt_scheduler.prior_stk_idx--;
    } else if (_interrupt_scheduler.prior_stk_idx == -1) {
        //is not running
        while (_interrupt_scheduler.qsize) {
            struct interrupt_task_node *task = fetch_next(&_interrupt_scheduler, &prior);
            _interrupt_scheduler.priority_stack[++(_interrupt_scheduler.prior_stk_idx)] = prior;
            test_enable_interrupt();
            //run single task
            task->handler(task->data);
            disable_interrupt();
            _interrupt_scheduler.prior_stk_idx--;
        }
    } //else {
        //other one is running, that guy will wake and handle the newly enqueued task
    //}
    test_enable_interrupt();
    check_reschedule();
}

void current_irq_exception_router(void)
{
    // uart_write_string("In current_irq_exception_router\n");
    irq_router();
}

static void syscall_handler(struct trap_frame *tf)
{
    uint64_t syscall_index = tf->gprs[8];
    task_t *current = get_current_thread();
    current->tf = tf;
    if (syscall_index < NUM_syscalls) {
        (default_syscall_table[syscall_index])(tf);
    } else {
        uart_write_string("Invalid syscall index!\n");
    }
}



void synchronous_exception_router(struct trap_frame *tf)
{
    //https://grasslab.github.io/osdi/en/labs/lab3.html#on-excpetion-taken
    //Interrupt is disabled. (PSTATE.{D,A,I,F} are set to 1).
    //to enable kernel preemption, enable interrupt
    if (interrupt_cnter == 0) enable_local_all_interrupt();
    // enable_local_all_interrupt();

    // uart_write_string("In lower_synchronous_exception_router\n");
    unsigned long esr = read_sysreg(esr_el1); // cause of that exception
    unsigned int ec = ESR_ELx_EC(esr);
    //https://developer.arm.com/documentation/ddi0601/2020-12/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-
    switch (ec)
    {
    case ESR_ELx_EC_SVC64:
        syscall_handler(tf);
        break;
    case ESR_ELx_EC_DABT_LOW:
        uart_write_string("in Data Abort\n");
        break;
    case ESR_ELx_EC_IABT_LOW:
        uart_write_string("in Instruction  Abort\n");
        break;
    default:
        return;
    }
}

void current_synchronous_exception_router(struct trap_frame *tf)
{
    // uart_write_string("In current_synchronous_exception_router\n");
    // default_handler();
    synchronous_exception_router(tf);
}

void lower_synchronous_exception_router(struct trap_frame *tf)
{
    // uart_write_string("In lower_synchronous_exception_router\n");
    synchronous_exception_router(tf);
}

void lower_irq_exception_router(void)
{
    // uart_write_string("In lower_irq_exception_router\n");
    // default_handler();
    irq_router();
}

void _insert_qbin(struct interrupt_scheduler *self, list_t *head, struct interrupt_task_node *task)
{
    list_add_tail(&(task->list), head);
    self->qsize++;
}

void _add_task(struct interrupt_scheduler *self, interrupt_handler_t handler, void *data, unsigned priority)
{
    //avoid re-entrance
    struct interrupt_task_node *new_node = (struct interrupt_task_node *)simple_malloc(sizeof(struct interrupt_task_node));
    new_node->handler = handler;
    new_node->data = data;
    INIT_LIST_HEAD(&(new_node->list));
    disable_interrupt();
    _insert_qbin(self, self->qbins + priority, new_node);
    test_enable_interrupt();
}

void init_interrupt_scheduler(struct interrupt_scheduler *self)
{
    self->prior_stk_idx = -1;
    for (int i = 0; i < PRIORITY_BINS; i++) {
        INIT_LIST_HEAD(self->qbins + i);
        self->priority_stack[i] = NULL;
    }
    self->qsize = 0;
    self->add_task = _add_task;
}

