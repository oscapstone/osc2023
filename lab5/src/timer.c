#include "mini_uart.h"

void core_timer_enable()
{
	asm volatile
	(
		"mov     x0,1;"
    	"msr     CNTP_CTL_EL0,x0;"	//bit0 : timer enabled , bit1 : IMASK(1: won't trigger interrupt)
    	"mrs     x0,CNTFRQ_EL0;"	//frequency of the system
		"asr	 x0,x0,5;"
    	"msr     CNTP_TVAL_EL0,x0;"	//on a write of this register , CNTP_CVAL_EL0 is set to (CNTPCT_EL0 + timer value)
									//timer value for thr EL1 physical timer , if(CNTP_CTL_EL0[0] is 1)->timer met when (CNTPCT_EL0-CNTP_CVAL_EL0) >= 0
		"ldr     x1,=0x40000040;"	//core0 timers interrupt control address : 0x40000040
		"mov     x0,2;"				//bit1 : IRQ enable
		"str     w0,[x1];"
	);
	uint64_t tmp;
	asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
	tmp |= 1;
	asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));

	return;
}

void core_timer_disable()
{
	asm volatile
	(
		"mov     x0,0;"
    	"msr     CNTP_CTL_EL0,x0;"     //bit0 : timer output signal enabled
    	"ldr     x1,=0x40000040;"      //core0 timers interrupt control address : 0x40000040
    	"mov     x0,0;"                //bit1 : IRQ disable
    	"str     w0,[x1];"
	);
	return;
}

void print_time()
{
    char* time;
    char* freq;
    asm volatile
    (   
        "mrs %0,CNTPCT_EL0;"
        "mrs %1,CNTFRQ_EL0;" : "=r" (time) , "=r" (freq)
    );  
    uart_send_string("core timeout interrupt : ");
    uart_hex((unsigned int)time / (unsigned int)freq);      //second : time/freq
    uart_send_string(" s\r\n");
    return;
}

typedef struct
{
	int second;
	void (*func_arg)(char*);
	void (*func)();
	int with_arg;
	char* message;
	int valid;				//can't use bool type
}timer;

timer t_queue[100];
int id = 0;

void init_t_queue()
{
	for(int i=0;i<100;i++)
	{
		t_queue[i].valid = 0;
	}
	return;
}

void add_timer_arg(void (*func)(char*),int second,int with_arg,char* message)
{
	t_queue[id].second = second;
	t_queue[id].func_arg = func;
	t_queue[id].with_arg = with_arg;
	t_queue[id].message = message;
	t_queue[id].valid = 1;
	id++;
	return;
}

void add_timer(void (*func)(),int second,int with_arg)
{
	t_queue[id].second = second;
	t_queue[id].func = func;
	t_queue[id].with_arg = with_arg;
	t_queue[id].valid = 1;
	id++;
	return;
}

void setTimeout(char* message,int second)
{
	void (*func)(char*);
	func = uart_send_string;
	add_timer_arg(func,second,1,message);			//add_timer with argument
	return;
}

void sleep(void (*func)(),int second)
{
	add_timer(func,second,0);
	return;	
}

void one_sec_pass()
{
	for(int i=0;i<id;i++)
	{
		if(t_queue[i].valid)
		{
			t_queue[i].second--;
			if(t_queue[i].second <= 0)
			{
				if(t_queue[i].with_arg)
				{
					t_queue[i].func_arg(t_queue[i].message);	//do function with message
				}
				else
				{
					t_queue[i].func();							//do function without argument
				}
				t_queue[i].valid = 0;
			}
		}
	}
	return;
}

