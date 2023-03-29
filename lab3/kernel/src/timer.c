#include "timer.h"
#include "uart1.h"
#include "heap.h"
#include "u_string.h"

#define STR(x) #x
#define XSTR(s) STR(s)

struct list_head* timer_event_list;  // first head has nothing, store timer_event_t after it 

void timer_list_init(){
    INIT_LIST_HEAD(timer_event_list);
}

void core_timer_enable(){
    __asm__ __volatile__(          //cntp_ctl_el0 : 打開或關閉timer count
        "mov x1, 1\n\t"
        "msr cntp_ctl_el0, x1\n\t" // cntp_ctl_el0[0]: enable, Control register for the EL1 physical timer.
                                   // cntp_tval_el0: Holds the timer value for the EL1 physical timer
        "mov x2, 2\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t" //ldr x1, =CORE0_TIMER_IRQ_CTRL #define CORE0_TIMER_IRQ_CTRL 0x40000040
        "str w2, [x1]\n\t"         // unmask timer interrupt
                                   // QA7_rev3.4.pdf: Core0 Timer IRQ allows Non-secure physical timer(nCNTPNSIRQ)
    );
}

void core_timer_disable()
{
    __asm__ __volatile__(
        "mov x2, 0\n\t"
        "ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n\t"
        "str w2, [x1]\n\t"         // QA7_rev3.4.pdf: Mask all timer interrupt
    );
}

void core_timer_handler(){
    if (list_empty(timer_event_list))
    {
        set_core_timer_interrupt(10000); // disable timer interrupt (set a very big value)
        return;
    }
    timer_event_callback((timer_event_t *)timer_event_list->next); // do callback and set new interrupt
}

void timer_event_callback(timer_event_t * timer_event){
    list_del_entry((struct list_head*)timer_event); // delete the event in queue
    free(timer_event->args);                        // free the event's space
    free(timer_event);
    ((void (*)(char*))timer_event-> callback)(timer_event->args);  // call the event
    //define a function ,void func1(int int1, char char1); 
    //define a function pointer,void (*funcPtr1)(int, char);
    //((void (*)(char*)) function pointer缺少name,address = timer_event-> callback,compiler會把address讀成function name,(timer_event->args)傳入function的參數,

    // set queue linked list to next time event if it exists
    if(!list_empty(timer_event_list))
    {
        set_core_timer_interrupt_by_tick(((timer_event_t*)timer_event_list->next)->interrupt_time);
    }
    else
    {
        set_core_timer_interrupt(10000);  // disable timer interrupt (set a very big value)
    }
}

void timer_set2sAlert(char* str)
{
    unsigned long long cntpct_el0;
    __asm__ __volatile__("mrs %0, cntpct_el0\n\t": "=r"(cntpct_el0)); // tick auchor
    unsigned long long cntfrq_el0;
    __asm__ __volatile__("mrs %0, cntfrq_el0\n\t": "=r"(cntfrq_el0)); // tick frequency
    uart_sendline("[Interrupt][el1_irq][%s] %d seconds after booting\n", str, cntpct_el0/cntfrq_el0);
    add_timer(timer_set2sAlert,2,"2sAlert");
}


void add_timer(void *callback, unsigned long long timeout, char* args){
    timer_event_t* the_timer_event = kmalloc(sizeof(timer_event_t)); // free by timer_event_callback
    // store all the related information in timer_event
    the_timer_event->args = kmalloc(strlen(args)+1);
    strcpy(the_timer_event -> args,args);
    the_timer_event->interrupt_time = get_tick_plus_s(timeout);
    the_timer_event->callback = callback;
    INIT_LIST_HEAD(&the_timer_event->listhead);

    // add the timer_event into timer_event_list (sorted)
    struct list_head* curr;
    list_for_each(curr,timer_event_list)
    {
        if(((timer_event_t*)curr)->interrupt_time > the_timer_event->interrupt_time)
        {
            list_add(&the_timer_event->listhead,curr->prev);  // add this timer at the place just before the bigger one (sorted)
            break;
        }
    }
    // if the timer_event is the biggest, run this code block
    if(list_is_head(curr,timer_event_list))
    {
        list_add_tail(&the_timer_event->listhead,timer_event_list);
    }
    // set interrupt to first event
    set_core_timer_interrupt_by_tick(((timer_event_t*)timer_event_list->next)->interrupt_time);
}


// This function calculates and returns the tick value that corresponds to a given number of seconds.
// It first retrieves the current value of the physical counter register (cntpct_el0), which serves as an anchor point for the calculation.
// It then retrieves the frequency of the physical timer (cntfrq_el0) to convert seconds to ticks.
// The returned value is the sum of the anchor point and the product of the frequency and the given number of seconds.
unsigned long long get_tick_plus_s(unsigned long long second){
    unsigned long long cntpct_el0=0;
    __asm__ __volatile__("mrs %0, cntpct_el0\n\t": "=r"(cntpct_el0)); // Retrieve current value of physical counter register (cntpct_el0)
    unsigned long long cntfrq_el0=0;
    __asm__ __volatile__("mrs %0, cntfrq_el0\n\t": "=r"(cntfrq_el0)); // Retrieve frequency of physical timer (cntfrq_el0)
    return (cntpct_el0 + cntfrq_el0*second); // Return the sum of the anchor point and the product of the frequency and the given number of seconds
    //The kernel can use the tick period as the time unit
    //suppose the periodic timer’s frequency is 1000HZ and a process sleeps for 1.5 seconds. The kernel can add a wake-up event at the moment that 1500 ticks after the current tick.
}

// set timer interrupt time to [expired_time] seconds after now (relatively)
void set_core_timer_interrupt(unsigned long long expired_time){
    __asm__ __volatile__(
        "mrs x1, cntfrq_el0\n\t"    // cntfrq_el0 -> frequency of the timer
        "mul x1, x1, %0\n\t"        // cntpct_el0 = cntfrq_el0 * seconds: relative timer to cntfrq_el0
        "msr cntp_tval_el0, x1\n\t" // Set expired time to cntp_tval_el0, which stores time value of EL1 physical timer.
    :"=r" (expired_time));
}

// directly set timer interrupt time to a cpu tick  (directly)
void set_core_timer_interrupt_by_tick(unsigned long long tick){
    __asm__ __volatile__(
        "msr cntp_cval_el0, %0\n\t"  //cntp_cval_el0 -> absolute timer
    :"=r" (tick));
}

// get timer pending queue size
int timer_list_get_size(){
    int r = 0;
    struct list_head* curr;
    list_for_each(curr,timer_event_list)
    {
        r++;
    }
    return r;
}
