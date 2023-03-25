#include "timer.h"
#include "mini_uart.h"

unsigned long get_current_time(void)
{
        unsigned long t;
        asm volatile("mrs %0, cntpct_el0": "=r"(t));
        return t;
}

unsigned long get_cpu_frequency(void)
{
        unsigned long f;
        asm volatile("mrs %0, cntfrq_el0": "=r"(f));
        return f;
}

void reset_core_timer_in_second(unsigned int sec)
{
        unsigned long cycle = get_cpu_frequency() * sec;
        reset_core_timer_in_cycle(cycle);
}

void print_current_time(void)
{
        unsigned long cycle = get_current_time();
        unsigned long freq = get_cpu_frequency();

        uart_send_string("CPU frequency:\t");
        uart_send_int(freq);
        uart_endl();
        uart_send_string("Current time:\t");
        uart_send_int(cycle/freq);
        uart_send_string(" second(s)\r\n");
}