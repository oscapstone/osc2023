#include "mini_uart.h"
#include "string.h"
#include "mbox.h"
#include "reboot.h"
#include "ramdisk.h"
#include "to_EL0.h"
#include "check_interrupt.h"
#include "timer.h"
#include "buddy.h"
#include "thread.h"
#include "system_call.h"

extern void from_EL1_to_EL0();

void shell_input(char* command)
{
	uart_send_string("# ");
	char c;
	int index=0;
	command[0] = '\0';
	while((c = uart_recv()) != '\n')
	{
		command[index++] = c;
		command[index] = '\0';
		uart_send(c);
	}
	uart_send_string("\r\n");
	return;
}

void shell_option(char* command,char* ramdisk)
{
	if(!strcmp(command,""))
	{
		return;
	}
	else if(!strcmp(command,"help"))
	{
		uart_send_string("help\t\t: print this help menu\r\n");
		uart_send_string("hello\t\t: print Hello world!\r\n");
		uart_send_string("reboot\t\t: reboot the device\r\n");
		uart_send_string("board\t\t: show board revision\r\n");
		uart_send_string("ls\t\t: show file name\r\n");
		uart_send_string("cat\t\t: show file content\r\n");
		uart_send_string("alloc\t\t: show allocate example\r\n");
		uart_send_string("timer\t\t: show timer-multiplexing process\r\n");
		uart_send_string("thread\t\t: show basic1\r\n");
		uart_send_string("exec\t\t: show basic2\r\n");
	}
	else if(!strcmp(command,"hello"))
	{
		uart_send_string("Hello World!\r\n");
	}
	else if(!strcmp(command,"reboot"))
	{
		uart_send_string("raspi3 is rebooting...\r\n");
		reset(200);
		while(1){}
	}
	else if(!strcmp(command,"board"))
	{
		get_board_revision();
		get_mem_info();
	}
	else if(!strcmp(command,"ls"))
	{
		ls();
	}
	else if(!strcmp(command,"cat"))
	{
		cat();
	}
	else if(!strcmp(command,"alloc"))
	{
		char* string = memalloc(8);
		uart_hex(string);
		uart_send_string("\r\n");
		char* string2 = memalloc(5);
		uart_hex(string2);
		uart_send_string("\r\n");
	}
	else if(!strcmp(command,"timer"))
	{
		enable_interrupt();										//interrupt mask clear
		core_timer_enable();									//start time
		init_t_queue();
		setTimeout("this will print after 6 second\r\n",6);		//should appear 6 second later
		setTimeout("this will print after 4 second\r\n",4);		//should appear 4 second later
		void (*func)();
		func = core_timer_disable;
		sleep(func,12);											//time will end in (start time + 12) second later
		func = disable_interrupt;
		sleep(func,12);											//interrupt mask set
	}
	else if(!strcmp(command,"thread"))
	{
		void (*fun)();
		fun = &foo;
		for(int i=0;i<3;i++)
		{
			Thread(fun);
		}
		idle();
	}
	else if(!strcmp(command,"exec"))
	{
		from_EL1_to_EL0();
		Thread(fork_test);
		schedule();
	}
	else
	{
		uart_send_string("unvalid command! try <help>\r\n");
	}
	return;
}
