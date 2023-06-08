#include "timer.h"
#include "uart.h"
#include "malloc.h"
#include "string.h"

#define STR(x) #x
#define XSTR(s) STR(s) 

struct list_head* timer_event_list;

void timer_list_init(){
	INIT_LIST_HEAD(timer_event_list);
}

void core_timer_enable(){
	unsigned long long cntp_ctl_el0;
	__asm__ __volatile__("mrs %0, cntp_ctl_el0\n":"=r"(cntp_ctl_el0));
	//uart_sendline("ctl %d \n", cntp_ctl_el0);

	__asm__ __volatile__(
		"mov x1, 1\n"
		"msr cntp_ctl_el0, x1\n" //cntp_ctl_el0: 1 enable EL1 physical timer.
		"mov x2, 2\n" 
		"ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n"  //ldr x1, =CORE0_TIMER_IRQ_CTRL #define CORE0_TIMER_IRQ_CTRL 0x40000040
		"str w2, [x1]\n" // unmask timer interrupt
	);

	unsigned long long cntpct_el0;
	__asm__ __volatile__("mrs %0, cntpct_el0\n":"=r"(cntpct_el0)); //get cntpct_el0
	unsigned long long cntfrq_el0;
	__asm__ __volatile__("mrs %0, cntfrq_el0\n":"=r"(cntfrq_el0)); //get cntfrq_el0
	/*
	uart_sendline("[Enable Timer interrupt] %d seconds after booting.\n", cntpct_el0/cntfrq_el0);
	
	uart_sendline("cntpct %d cntfrq %d", cntpct_el0,cntfrq_el0);
	__asm__ __volatile__("mrs %0, cntp_ctl_el0\n":"=r"(cntp_ctl_el0));
	uart_sendline("ctl %d \n", cntp_ctl_el0);
	*/
}


void core_timer_disable(){
	__asm__ __volatile__(
		"mov x2, 0\n"
		"ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n"
		"str w2, [x1]\n" //mask timer interrupt
	);
}

void core_timer_handler(timer_event_t *timer_event){ //when trigger timer exception, 
	//uart_sendline("===in core timer handler===\n");
    if (list_empty(timer_event_list))
    {
        set_core_timer_interrupt(10000); // disable timer interrupt (set a very big value)
        //uart_sendline("==disable timer interrupt==\n");
        return;
    }
    timer_event_callback((timer_event_t *)timer_event_list->next); // do callback and set new interrupt
    //uart_sendline("===end core timer handler===\n");
}


void timer_event_callback(timer_event_t * timer_event){
    list_del_entry((struct list_head*)timer_event); // delete the event in queue

    ((void (*)(char*))timer_event-> callback)(timer_event->args);  // call the event

    // set queue linked list to next time event if it exists
    if(!list_empty(timer_event_list))
    {
        set_core_timer_interrupt_by_tick(((timer_event_t*)timer_event_list->next)->interrupt_time);
    }
    else
    {
        set_core_timer_interrupt(10000);  // disable timer interrupt (set a very big value)
        //uart_sendline("==disable timer interrupt==\n");
    }
}

//implement basic 2
void timer_2s(){
	unsigned long long cntpct_el0;
	__asm__ __volatile__("mrs %0, cntpct_el0\n":"=r"(cntpct_el0)); //get cntpct_el0
	unsigned long long cntfrq_el0;
	__asm__ __volatile__("mrs %0, cntfrq_el0\n":"=r"(cntfrq_el0)); //get cntfrq_el0
	uart_sendline("[Timer interrupt] %d seconds after booting., cntpct:%d, cntfrq: %d\n", cntpct_el0/cntfrq_el0, cntpct_el0, cntfrq_el0);
	uart_sendline("%d \n",cntpct_el0 +2*cntfrq_el0);
	uart_sendline("%d \n",cntfrq_el0);
	set_core_timer_interrupt_by_tick(cntpct_el0 +2*cntfrq_el0);
	add_timer(timer_2s,2,"basic 2");
}

// set timer interrupt time to [expired_time] seconds after now (relatively)
void set_core_timer_interrupt(unsigned long long etime){
    __asm__ __volatile__(
        "mrs x1, cntfrq_el0\n"    // cntfrq_el0: frequency of the timer
        "mul x1, x1, %0\n"        // cntpct_el0 = cntfrq_el0 * seconds: relative timer to cntfrq_el0
        "msr cntp_tval_el0, x1\n" // Set expired time to cntp_tval_el0, 會把cval設為now(cntpct)+tval 
    :"=r" (etime));
}

// set timer interrupt time to a cpu tick  (directly) 
void set_core_timer_interrupt_by_tick(unsigned long long tick){
    __asm__ __volatile__(
        "msr cntp_cval_el0, %0\n\t"  //cntp_cval_el0  (when cntpct >= cval, trigger timer interrupt)
    :"=r" (tick));
}


void add_timer(void *callback, unsigned long long timeout, char* args){
    timer_event_t* the_timer_event = malloc(sizeof(timer_event_t)); 

    //information in timer_event
    the_timer_event->args = malloc(strlen(args)+1);
    strcpy(the_timer_event -> args,args);
    the_timer_event->interrupt_time = get_tick_plus_s(timeout);
    the_timer_event->callback = callback;
    
    INIT_LIST_HEAD(&the_timer_event->listhead);

    // add the timer_event into timer_event_list (sorted)
    struct list_head* curr;
    list_for_each(curr,timer_event_list) //curr list & global timer_event_list 
    {
        if(((timer_event_t*)curr)->interrupt_time > the_timer_event->interrupt_time) 
        {
            list_add(&the_timer_event->listhead,curr->prev);  // add this timer at the place just before the bigger one (sorted)
            break;
        }
    }
    // if the timer_event is the biggest, add at tail 
    if(list_is_head(curr,timer_event_list))
    {
        list_add_tail(&the_timer_event->listhead,timer_event_list);
    }
    // set interrupt to first event
    set_core_timer_interrupt_by_tick(((timer_event_t*)timer_event_list->next)->interrupt_time);
}


// calculate cpu tick after n second from now 
unsigned long long get_tick_plus_s(unsigned long long second){
    unsigned long long cntpct_el0=0;
    __asm__ __volatile__("mrs %0, cntpct_el0\n\t": "=r"(cntpct_el0)); // now
    unsigned long long cntfrq_el0=0;
    __asm__ __volatile__("mrs %0, cntfrq_el0\n\t": "=r"(cntfrq_el0)); // tick frequency
    return (cntpct_el0 + cntfrq_el0*second); 
}