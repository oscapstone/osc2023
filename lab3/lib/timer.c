#include "timer.h"
#include "utils.h"
#include "memory.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "exception.h"
#include "peripherals/exception.h"

static struct timer_cb *timer_list = 0;

void core_timer_enable() {

    asm volatile(
        "mov x0, 1\n\t"
        "msr cntp_ctl_el0, x0\n\t" // enable; set cntp_ctl_el0 to 1
        "mov x0, 2\n\t"
        "ldr x1, =0x40000040\n\t" // CORE0_TIMER_IRQ_CTRL
        "str w0, [x1]\n\t" // unmask timer interrupt
    );

}

void core_timer_disable() {

    asm volatile(
        "mov x0, 0\n\t"
        "ldr x1, =0x40000040\n\t"
        "str w0, [x1]\n\t"
    );

}

void set_timer(unsigned int rel_time) {

    asm volatile(
        "msr cntp_tval_el0, %0\n\t"     //(cntp_cval_el0 - cntpct_el0)
        :
        : "r" (rel_time)
    );

}

unsigned int read_timer() {

    unsigned int time;
    asm volatile("mrs %0, cntpct_el0\n\t" : "=r" (time) :  : "memory");    //timer count in second
    return time;

}

unsigned int read_freq() {

    unsigned int freq;
    asm volatile("mrs %0, cntfrq_el0\n\t": "=r" (freq) : : "memory");       //timer frequency
    return freq;

}
 
void add_timer(void (*func)(void*), void* arg, unsigned int time) {
    struct timer_cb *timer = (struct timer_cb*)simple_malloc(sizeof(struct timer_cb));
    timer->expire_time = ((unsigned int)get32(CNTPCT_EL0)) + time;
    timer->timer_callback = func;
    timer->arg = arg;
    timer->next = 0;
    int update = 0;
    if (!timer_list) {
        //add timer to empty list
        timer_list = timer;
        update = 1;
    }
    else if (timer_list->expire_time > timer->expire_time) {
        //add to head of list
        timer->next = timer_list;
        timer_list = timer;
        update = 1;
    }
    else {
        struct timer_cb *prev = timer_list, *next = timer_list->next;
        //find proper location to insert
        //where time1<current timer<time3
        while (next && prev->expire_time < timer->expire_time) {
            prev = next;
            next = next->next;
        }
        prev->next = timer;
        timer->next = next;
    }
    
    if (update)
        set_timer(time); // set next interrupt

}

void pop_timer() {

    struct timer_cb *timer = timer_list;
    timer_list = timer_list->next;
    timer->timer_callback(timer->arg);
    if (!timer_list) {
        core_timer_disable();
    }
    else {
        unsigned int cur_time = read_timer();
        set_timer(timer_list->expire_time-cur_time); // set next interrupt
    }
        

}

void test_multiplex() {
    disable_interrupt();
    char *msg1 = (char*)simple_malloc(32);
    char *msg2 = (char*)simple_malloc(32);
    char *msg3 = (char*)simple_malloc(32);
    msg1 = "1st sent, 3rd print.\n";
    msg2 = "2nd sent, 2nd print.\n";
    msg3 = "\n3rd sent, 1st print.\n";
    add_timer(print_timer, msg1, 3*read_freq());
    add_timer(print_timer, msg2, 2*read_freq());
    add_timer(print_timer, msg3, 1*read_freq());
    enable_interrupt();
    core_timer_enable();
}

void ct_enable_fortt() {
    //infinity time
    while (1);

}

void two_test() {

    disable_interrupt();
    core_timer_enable();

    asm volatile(
        "mrs x0, cntfrq_el0\n\t"
        "mov x1, 2\n\t"
        "mul x0, x0, x1\n\t"
        "msr cntp_tval_el0, x0\n\t"
    );

    void (*ct_enable)(void) = ct_enable_fortt; 

    asm volatile(
        "msr elr_el1, %0\n\t"
        "mov x0, 0x340\n\t"
        "msr spsr_el1, x0\n\t"
        "mov x0, sp\n\t"
        "msr sp_el0, x0\n\t"
        "eret\n\t"
        :: 
        "r" (ct_enable)
    );

    while (1);

}

void time_elapsed() {

    unsigned int sec;

    //seconds after booting
    unsigned int cntpct_el0;
    asm volatile("mrs %0, CNTPCT_EL0\n\t" : "=r" (cntpct_el0) :  : "memory");

    //frequency of the timer
    unsigned int cntfrq_el0;
    asm volatile("mrs %0, CNTFRQ_EL0\n\t" : "=r" (cntfrq_el0) :  : "memory");

    sec = cntpct_el0 / cntfrq_el0;      //total time=count/freq

    uart_send_string("hex-format seconds after boot:");
    printhex(sec);
    uart_send_string("\n");

}

void print_timer(void *arg) {

    char *msg = (char*)arg;
    uart_send_string(msg);

}