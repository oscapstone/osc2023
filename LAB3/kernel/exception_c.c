#include "utils_s.h"
#include "mini_uart.h"
#include "timer.h"
#include "peripheral/mini_uart.h"
#include "exception_c.h"
#include "allocator.h"
#define AUX_IRQ (1 << 29)   //the second level interrupt controller’s Enable IRQs1(0x3f00b210)’s bit29.


int doing_task = 0;

task *task_queue_head = 0, *task_queue_tail = 0;

//required to enable interrupts in EL1. 
//You can only disable interrupts to protect the critical sections.
void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }


void default_handler()
{
    disable_interrupt();
    unsigned long spsr = read_sysreg(spsr_el1);
    unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    uart_send_string("-----default handler-----\n");
    uart_printf("spsr_el1: %x\n", spsr);
    uart_printf("elr_el1: %x\n", elr);
    uart_printf("esr_el1: %x\n\n", esr);
    enable_interrupt();
}

void lower_irq_handler()
{
    unsigned long current_time = get_current_time();
    uart_printf("After booting: %d seconds\n\n", current_time);
    uart_printf("set_expired_time(2).....");
    set_expired_time(2);
}//沒用到？？？

void lower_sync_handler()
{
    disable_interrupt();
    unsigned long spsr = read_sysreg(spsr_el1);
    unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    uart_send_string("-----lower_sync_handler-----\n");
    uart_printf("spsr_el1: %x\n", spsr);
    uart_printf("elr_el1: %x\n", elr);
    uart_printf("esr_el1: %x\n\n", esr);
    enable_interrupt();
}

void curr_irq_handler()
{
    //     disable_interrupt();
    //     unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    //     unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    //     if (uart)
    //     {
    //         // uart_handler();
    //     }
    //     else if (core_timer)
    //     {
    //         unsigned long current_time = get_current_time();
    //         uart_send_string("\nmessage :");
    //         timeout_queue_head->callback(timeout_queue_head->msg);
    //         uart_printf("\ncurrent time : %ds\n",current_time);
    //         uart_printf("command executed time : %ds\n",timeout_queue_head->register_time);
    //         uart_printf("command duration time : %ds\n\n",timeout_queue_head->duration);

    //         timeout_event *next = timeout_queue_head->next;
    //         if (next)
    //         {
    //             next->prev = 0;
    //             timeout_queue_head = next;
    //             set_expired_time(next->register_time + next->duration - get_current_time());
    //         }
    //         else // no other event
    //         {
    //             timeout_queue_head = timeout_queue_tail = 0;
    //             core_timer_disable();
    //         }
    //     }
    //     enable_interrupt();
}


//  Concurrent I/O Devices Handling 

//  Usually, we want to use the first come first serve principle to prevent starvation. 
//  However, we may also want prioritized execution for some critical handlers. 
//  In this part, you need to know how to implement it using a single thread(i.e. a single stack).
void add_task(task_callback cb, void *arg, unsigned int priority)
{
    task *new_task = (task *)malloc(sizeof(task));
    new_task->priority = priority;
    new_task->callback = cb;
    new_task->arg = arg;
    new_task->next = 0;
    new_task->prev = 0;
    if (task_queue_head == 0)
    {
        task_queue_head = new_task;
        task_queue_tail = new_task;
    }
    else
    {
        task *cur = task_queue_head;
        // while (cur)
        // {
        //     cur = cur->next;
        // }
        // task_queue_tail->next = new_task;
        // new_task->prev = task_queue_tail;
        // task_queue_tail = new_task;
        while (cur)
        {
            if (cur->priority < new_task->priority)
                break;
            cur = cur->next;
        }
        if (cur == 0)  //cur == 0 == task_queue_tail->next
        { // cur at end, 找到剩最後一個人後面才有空位了
            new_task->prev = task_queue_tail;
            task_queue_tail->next = new_task;
            task_queue_tail = new_task;
            //並且task_queue_tail->next沒設定，所以等於0
        }
        else if (cur->prev == 0)   
        { // cur at head
            new_task->next = cur;
            (task_queue_head)->prev = new_task;
            task_queue_head = new_task;
            //並且task_queue_head->prev沒設定，所以等於0
        }
        else
        { // cur at middle
            new_task->next = cur;
            new_task->prev = cur->prev;
            (cur->prev)->next = new_task;
            cur->prev = new_task;
        }
    }
}

void exec_task()
{
    while (1)
    {
        task_queue_head->callback(task_queue_head->arg);  //???明明沒收參數？
        disable_interrupt();
        task_queue_head = task_queue_head->next;
        if (task_queue_head)
        {
            task_queue_head->prev = 0;
        }
        else
        {
            task_queue_head = task_queue_tail = 0;
            return;
        }
        enable_interrupt();
    }
}


void curr_irq_handler_decouple()
{
    // Determine the Interrupt Source
    // 檢查是否有UART中斷
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    // 檢查是否有core timer中斷
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    
    // 如果有UART中斷
    if (uart)
    {
        // 分配一塊記憶體用於儲存中斷處理函式需要使用的暫存器
        Reg *reg = (Reg *)malloc(sizeof(Reg));
        *reg = *AUX_MU_IER_REG;
        // 將一個新的任務（task）加入到任務列表中，並指定中斷處理函式為uart_handler
        add_task(uart_handler, reg, 3);
        // 禁用UART中斷
        *AUX_MU_IER_REG &= ~(0x3);
    }
    // 如果有core timer中斷
    else if (core_timer)
    {
        // 將一個新的任務（task）加入到任務列表中，並指定中斷處理函式為timer_handler
        add_task(timer_handler, NULL, 0);
        // 禁用core timer中斷
        core_timer_disable();
    }
    
    // 如果目前沒有執行任何任務
    if (!doing_task)
    {
        // 設置doing_task標誌為1，表示正在執行任務
        doing_task = 1;
        // 啟用中斷
        enable_interrupt();
        // 執行下一個任務
        exec_task();
        // 再次啟用中斷
        enable_interrupt();
        // 設置doing_task標誌為0，表示任務已經執行完畢
        doing_task = 0;
    }
}

// void curr_irq_handler_decouple()
// {
//     unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
//     unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
//     if (uart)
//     {
//         Reg *reg = (Reg *)malloc(sizeof(Reg));  //typedef unsigned int Reg;
//         *reg = *AUX_MU_IER_REG;
//         add_task(uart_handler, reg, 3);
//         *AUX_MU_IER_REG &= ~(0x3);
//     }
//     else if (core_timer)
//     {
//         add_task(timer_handler, NULL, 0);
//         core_timer_disable();
//     }
//     if (!doing_task)
//     {
//         doing_task = 1;
//         enable_interrupt();
//         exec_task();
//         enable_interrupt();
//         doing_task = 0;
//     }
// }
void curr_sync_handler(unsigned long esr_el1, unsigned long elr_el1)
{
    disable_interrupt();
    return;
    enable_interrupt();
}
