#include "timer.h"
#include "mini_uart.h"
#include "string_utils.h"
#include "mem_utils.h"

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

/************************************
 *    For Timer Multiplexing API    *
 ************************************/

#define MAX_NUM_TIMER 16

struct timer {
        int in_use;
        unsigned long expire_cycle;
        void (*callback)(void*);
        void* arg;
};

static struct timer timer_list[MAX_NUM_TIMER];
static unsigned long next_expire_id = MAX_NUM_TIMER;

void set_next_expire(void)
{
        int set = 0;
        unsigned long next_expire_cycle = 0x7fffffffffffffffll;
        for (int i = 0; i < MAX_NUM_TIMER; i++) {
                if (!timer_list[i].in_use) continue;
                if (timer_list[i].expire_cycle >= next_expire_cycle) continue;

                next_expire_cycle = timer_list[i].expire_cycle;
                next_expire_id = i;
                set = 1;
        }
        if (!set) {
                next_expire_cycle = 0x7fffffffffffffffll;
                reset_core_timer_in_second(600);
                next_expire_id = MAX_NUM_TIMER;
                return;
        }

        unsigned long cycle = next_expire_cycle - get_current_time();
        reset_core_timer_in_cycle(cycle);
}

void el1_timer_handler(void)
{
        reset_core_timer_in_second(1);
        uart_send_string("wtf\r\n");
        int id = next_expire_id;
        if (id < MAX_NUM_TIMER) {
                timer_list[id].in_use = 0;
                timer_list[id].callback(timer_list[id].arg);
        }
        set_next_expire();
}

int find_space(void)
{
        int i;
        for (i = 0; i < MAX_NUM_TIMER && timer_list[i].in_use; i++) ;
        return i;
}

void demo_callback(void *str)
{
        uart_send_string("Timer callback msg: ");
        uart_send_string((char*) str);
        uart_endl();
}

void add_timer(void (*callback)(void*), void *arg, int sec)
{
        int id = find_space();
        if (id >= MAX_NUM_TIMER) {
                uart_send_string("[ERROR] No more space for new timer\r\n");
                return;
        }

        unsigned long num_cycle = sec * get_cpu_frequency();
        unsigned long exp_cycle = num_cycle + get_current_time();

        timer_list[id].expire_cycle = exp_cycle;
        timer_list[id].in_use = 1;
        timer_list[id].callback = callback;
        timer_list[id].arg = arg;

        set_next_expire();
        // if (exp_cycle < next_expire_cycle) {
        //         next_expire_cycle = exp_cycle;
        //         next_expire_id = id;
        //         reset_core_timer_in_cycle(num_cycle);
        // }
}

void cmd_add_timer(char* cmd)
{
        int i;
        for (i = strlen(cmd) - 1; cmd[i] != ' '; i--) ;
        cmd[i] = '\0';
        char* sec_str = cmd + i + 1;
        int sec = string_to_int(sec_str, strlen(sec_str));

        char* msg_in_cmd = cmd + strlen("set-timeout ");
        // TODO: free this
        int len = strlen(msg_in_cmd);
        char* msg = simple_malloc(len+1);
        memcpy(msg, msg_in_cmd, len);
        msg[len] = '\0';

        uart_send_string("[DEBUG]\r\n");
        uart_send_string("msg: ");
        uart_send_string(msg);
        uart_endl();
        uart_send_string("sec: ");
        uart_send_int(sec);
        uart_endl();
        uart_send_string("len: ");
        uart_send_int(len);
        uart_endl();

        add_timer(demo_callback, msg, sec);
}