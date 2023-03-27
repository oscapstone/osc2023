#include "mini_uart.h"

typedef struct
{
	int second;
	void (*func)(char*);
	char* message;
	int valid;				//can't use bool type
}timer;

timer t_queue[100];
int index = 0;

void init_t_queue()
{
	for(int i=0;i<100;i++)
	{
		t_queue[i].valid = 0;
	}
	return;
}

void add_timer(void (*func)(char*),int second,char* message)
{
	t_queue[index].second = second;
	t_queue[index].func = func;
	t_queue[index].message = message;
	t_queue[index].valid = 1;
	index++;
	return;
}

void setTimeout(char* message,int second)
{
	void (*func)(char*);
	func = uart_send_string;
	add_timer(func,second,message);
	return;
}

void one_sec_pass()
{
	for(int i=0;i<index;i++)
	{
		if(t_queue[i].valid)
		{
			t_queue[i].second--;
			if(t_queue[i].second <= 0)
			{
				t_queue[i].func(t_queue[i].message);		//do function pointer with message
				t_queue[i].valid = 0;
			}
		}
	}
	return;
}
