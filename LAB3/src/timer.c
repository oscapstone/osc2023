#include "timer.h"
#include "utils_s.h"
#include "allocator.h"
#include "mini_uart.h"
#include "utils_c.h"
timeout_event *timeout_queue_head, *timeout_queue_tail;

void core_timer_enable()
{
    /*
        cntpct_el0 >= cntp_cval_el0 -> interrupt
        cntp_tval_el0 = cntp_cval_el0 - cntpct_el0
    */
    write_sysreg(cntp_ctl_el0, 1); // enable
    unsigned long frq = read_sysreg(cntfrq_el0);
    write_sysreg(cntp_tval_el0, frq * 2); // set expired time 設定成兩倍頻率
    *CORE0_TIMER_IRQ_CTRL = 2;            // unmask timer interrupt
}

void core_timer_disable()
{
    write_sysreg(cntp_ctl_el0, 0); // disable
    *CORE0_TIMER_IRQ_CTRL = 0;     // unmask timer interrupt
}

void set_expired_time(unsigned long duration)
{
    unsigned long frq = read_sysreg(cntfrq_el0);
    write_sysreg(cntp_tval_el0, frq * duration); // set expired time
}

unsigned long get_current_time()
{
    // cntpct_el0: The timer’s current count.
    unsigned long frq = read_sysreg(cntfrq_el0);
    unsigned long current_count = read_sysreg(cntpct_el0);
    return (unsigned long)(current_count / frq);    //Sec
}

void set_timeout(char *message, char *_time)
{
    unsigned int time = utils_str2uint_dec(_time);
    add_timer(print_message, message, time);
}

void print_message(char *msg)
{
    uart_send_string(msg);
}

void timeout_event_init()
{
    timeout_queue_head = 0;
    timeout_queue_tail = 0;
}


//  Timer Multiplexing

// Another way is using a one-shot timer. When someone needs a timeout event, a timer is inserted into a timer queue. 
// If the timeout is earlier than the previous programed expired time, the kernel reprograms the hardware timer to the earlier one. 
// In the timer interrupt handler, it executes the expired timer’s callback function.
void add_timer(timer_callback cb, char *msg, unsigned long duration)
{
    // 建立一個新的 timeout_event 構造體，並用 malloc 函數為其分配記憶體空間
    timeout_event *new_event = (timeout_event *)malloc(sizeof(timeout_event));

    // 設置新事件的註冊時間為當前時間
    new_event->register_time = get_current_time();

    // 設置新事件的持續時間
    new_event->duration = duration;

    // 設置新事件的回調函數
    new_event->callback = cb;

    // 複製訊息字串到新事件的 msg 屬性中
    for (int i = 0; i < 20; i++)
    {
        new_event->msg[i] = msg[i];
        if (msg[i] == '\0')
            break;
    }

    // 設置新事件的 prev 和 next 屬性為 0，表示它還未插入到隊列中
    new_event->next = 0;
    new_event->prev = 0;

    // 如果事件隊列為空，將新事件設置為隊列頭和尾，並啟動核心定時器並設置過期時間
    if (timeout_queue_head == 0)
    {
        timeout_queue_head = new_event;
        timeout_queue_tail = new_event;
        // 為何設定兩次write_sysreg？？？？
        core_timer_enable();
        set_expired_time(duration);
    }
    else
    {
        // 計算新事件的過期時間
        unsigned long timeout= new_event->register_time + new_event->duration;

        // 在事件隊列中尋找適當的位置以插入新事件
        timeout_event *cur = timeout_queue_head;
        
        while (cur)
        {
            // 插入排序方式為『所有timeout event中timeout時間』小的在前，大的在後
            if ( (cur->register_time + cur->duration) > timeout)
                break;
            cur = cur->next;
        }

        // 如果 cur 為 0，表示新事件應該插入到隊列尾
        if (cur == 0)
        { // cur at end
            new_event->prev = timeout_queue_tail;
            timeout_queue_tail->next = new_event;
            timeout_queue_tail = new_event;


            // set_expired_time(duration);??????
        }
        // 如果 cur 的 prev 屬性為 0，表示新事件應該插入到隊列頭
        else if (cur->prev == 0)
        { // cur at head
            new_event->next = cur;
            (timeout_queue_head)->prev = new_event;
            timeout_queue_head = new_event;

            set_expired_time(duration);
        }
        // 其他情況表示新事件應該插入到 cur 和 cur->prev 之間
        else
        { // cur at middle
            new_event->next = cur;
            new_event->prev = cur->prev;
            (cur->prev)->next = new_event;
            cur->prev = new_event;


            // set_expired_time(duration);??????
        }
    }
}

// void add_timer(timer_callback cb, char *msg, unsigned long duration)
// {
//     timeout_event *new_event = (timeout_event *)malloc(sizeof(timeout_event));
//     new_event->register_time = get_current_time();
//     new_event->duration = duration;
//     new_event->callback = cb;
//     for (int i = 0; i < 20; i++)
//     {
//         new_event->msg[i] = msg[i];
//         if (msg[i] == '\0')
//             break;
//     }
//     new_event->next = 0;
//     new_event->prev = 0;

//     if (timeout_queue_head == 0)
//     {
//         timeout_queue_head = new_event;
//         timeout_queue_tail = new_event;
//         core_timer_enable();
//         set_expired_time(duration);
//     }
//     else
//     {
//         unsigned long timeout= new_event->register_time + new_event->duration;
//         timeout_event *cur = timeout_queue_head;
//         while (cur)
//         {
//             if ( (cur->register_time + cur->duration) > timeout)
//                 break;
//             cur = cur->next;
//         }
//         if (cur == 0)
//         { // cur at end
//             new_event->prev = timeout_queue_tail;
//             timeout_queue_tail->next = new_event;
//             timeout_queue_tail = new_event;
//         }
//         else if (cur->prev == 0)
//         { // cur at head
//             new_event->next = cur;
//             (timeout_queue_head)->prev = new_event;
//             timeout_queue_head = new_event;
//             set_expired_time(duration);
//         }
//         else
//         { // cur at middle
//             new_event->next = cur;
//             new_event->prev = cur->prev;
//             (cur->prev)->next = new_event;
//             cur->prev = new_event;
//         }
//     }
// }

void timer_handler(void *arg)
{
    // 取得目前時間
    unsigned long current_time = get_current_time();

    // 將字串 "message :" 傳送至 UART 裝置
    uart_send_string("\nmessage :");

    // 執行 timeout_queue_head 所指向的事件的回呼函式，將 timeout_queue_head 所指向的事件的訊息傳入
    timeout_queue_head->callback(timeout_queue_head->msg);

    // 將目前時間傳送至 UART 裝置
    uart_printf("\ncurrent time : %ds\n", current_time);

    // 將 timeout_queue_head 所指向的事件的註冊時間、執行時間以及持續時間分別傳送至 UART 裝置
    uart_printf("command executed time : %ds\n", timeout_queue_head->register_time);
    uart_printf("command duration time : %ds\n\n", timeout_queue_head->duration);

    // 取得 timeout_queue_head 的下一個事件指標
    timeout_event *next = timeout_queue_head->next;

    // 如果下一個事件存在
    if (next)
    {
        // 將下一個事件的前一個事件指標設為 0
        next->prev = 0;

        // 將 timeout_queue_head 設為下一個事件指標
        timeout_queue_head = next;

        // 啟用核心定時器
        core_timer_enable();

        // 設定過期時間為下一個事件的註冊時間加上持續時間減去目前時間
        set_expired_time(next->register_time + next->duration - get_current_time());
    }
    // 如果沒有其他事件
    else
    {
        // 將 timeout_queue_head 和 timeout_queue_tail 設為 0
        timeout_queue_head = timeout_queue_tail = 0;

        // 禁用核心定時器
        core_timer_disable();
    }
}
