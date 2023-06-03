#include "timer.h"
#include "uart.h"
//#include "heap.h"

#define STR(x) #x
#define XSTR(s) STR(s) 

/*struct list_head* timer_event_list;

void timer_list_init(){
	INIT_LIST_HEAD(timer_event_list);
}
*/

void core_timer_enable(){
	unsigned long long cntp_ctl_el0;
	__asm__ __volatile__("mrs %0, cntp_ctl_el0\n":"=r"(cntp_ctl_el0));
	uart_sendline("ctl %d \n", cntp_ctl_el0);

	__asm__ __volatile__(
		"mov x1, 1\n"
		"msr cntp_ctl_el0, x1\n"
		"mov x2, 2\n"
		"ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n"
		"str w2, [x1]\n"
	);
	unsigned long long cntpct_el0;
	__asm__ __volatile__("mrs %0, cntpct_el0\n":"=r"(cntpct_el0)); //get cntpct_el0
	unsigned long long cntfrq_el0;
	__asm__ __volatile__("mrs %0, cntfrq_el0\n":"=r"(cntfrq_el0)); //get cntfrq_el0
	uart_sendline("[Enable Timer interrupt] %d seconds after booting.\n", cntpct_el0/cntfrq_el0);
	uart_sendline("cntpct %d cntfrq %d", cntpct_el0,cntfrq_el0);

	__asm__ __volatile__("mrs %0, cntp_ctl_el0\n":"=r"(cntp_ctl_el0));
	uart_sendline("ctl %d \n", cntp_ctl_el0);

	unsigned long long cntp_cval_el0;
	__asm__ __volatile__("mrs %0, cntp_cval_el0\n":"=r"(cntp_cval_el0));
	uart_sendline("cval %d \n", cntp_cval_el0);
	set_core_timer_interrupt(2);
	//__asm__ __volatile__("mrs %0, cntp_cval_el0\n":"=r"(cntp_cval_el0));
	//uart_sendline("after set core cval %d \n", cntp_cval_el0);
	//set_core_timer_interrupt_by_tick(187500000);
	//add_timer(3);

}

void core_timer_disable(){
	__asm__ __volatile__(
		"mov x2, 0\n"
		"ldr x1, =" XSTR(CORE0_TIMER_IRQ_CTRL) "\n"
		"str w2, [x1]\n"
	);
}


void core_timer_handler(){
	//timer_event_callback();
	//set_core_timer_interrupt(1);
	__asm__ __volatile__(
		"mrs x0, cntfrq_el0\n"
  		"msr cntp_tval_el0, x0\n"
  	);
	uart_sendline("in core timer handler");

}
/*
void timer_event_callback(timer_event_t* timer_event){

}
*/

void timer_2s(){
	unsigned long long cntpct_el0;
	__asm__ __volatile__("mrs %0, cntpct_el0\n":"=r"(cntpct_el0)); //get cntpct_el0
	unsigned long long cntfrq_el0;
	__asm__ __volatile__("mrs %0, cntfrq_el0\n":"=r"(cntfrq_el0)); //get cntfrq_el0
	uart_sendline("[Timer interrupt] %d seconds after booting., cntpct:%d, cntfrq: %d\n", cntpct_el0/cntfrq_el0, cntpct_el0, cntfrq_el0);
	uart_sendline("%d \n",cntpct_el0 +2*cntfrq_el0);
	uart_sendline("%d \n",cntfrq_el0);
	set_core_timer_interrupt_by_tick(cntpct_el0 +2*cntfrq_el0);
	//uart_sendline("[Timer interrupt] %d seconds after booting.\n", cntpct_el0/cntfrq_el0);
	//timer_2s();
}



void set_core_timer_interrupt_by_tick(unsigned long long tick){
	__asm__ __volatile__(
		"msr cntp_cval_el0, %0\n" // cntp_cval_el0: get the seconds after booting
		:"=r" (tick));
	//uart_sendline("in tick");
	unsigned long long cval;
	__asm__ __volatile__("mrs %0, cntp_cval_el0\n":"=r"(cval));
	uart_sendline("print set core timer bt tick: %d\n", cval);
	uart_sendline("time %llu\n", tick);
}

// set timer interrupt time to [expired_time] seconds after now (relatively)
void set_core_timer_interrupt(unsigned long long etime){
    __asm__ __volatile__(
        "mrs x1, cntfrq_el0\n"    // cntfrq_el0: frequency of the timer
        "mul x1, x1, %0\n"        // cntpct_el0 = cntfrq_el0 * seconds: relative timer to cntfrq_el0
        "msr cntp_tval_el0, x1\n" // Set expired time to cntp_tval_el0, which stores time value of EL1 physical timer.
    :"=r" (etime));

    unsigned long long cval;
	__asm__ __volatile__("mrs %0, cntp_cval_el0\n":"=r"(cval));
	//uart_sendline("expired_time: %d, cval: %d \n", time,cntp_cval_el0);
	//uart_sendline("cval: %d \n", cval);
	uart_sendline("time %d\n",etime);


}

void add_timer(unsigned long long timeout){

    // set interrupt to first event
    set_core_timer_interrupt_by_tick(timeout);
    uart_sendline("print add_timer");
}

// get cpu tick add some second
unsigned long long get_tick_plus_s(unsigned long long second){
    unsigned long long cntpct_el0=0;
    __asm__ __volatile__("mrs %0, cntpct_el0\n\t": "=r"(cntpct_el0)); // tick auchor
    unsigned long long cntfrq_el0=0;
    __asm__ __volatile__("mrs %0, cntfrq_el0\n\t": "=r"(cntfrq_el0)); // tick frequency
    return (cntpct_el0 + cntfrq_el0*second);
}