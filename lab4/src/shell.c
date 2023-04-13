#include "mini_uart.h"
#include "string.h"
#include "mbox.h"
#include "reboot.h"
#include "ramdisk.h"
#include "to_EL0.h"
#include "check_interrupt.h"
#include "timer.h"
#include "buddy.h"

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
		uart_send_string("run\t\t: run an function in EL0\r\n");
		uart_send_string("async_io\t\t: show async_io process\r\n");
		uart_send_string("timer\t\t: show timer-multiplexing process\r\n");
		uart_send_string("p_alloc\t\t: alloc page by KB\r\n");
		uart_send_string("p_free\t\t: free page by index\r\n");
		uart_send_string("p_show\t\t: show page_array log\r\n");
		uart_send_string("d_alloc\t\t: alloc page by byte\r\n");
		uart_send_string("show_pool\t\t: show pool info\r\n");
		uart_send_string("clear_pool\t\t: clear pool\r\n");
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
	else if(!strcmp(command,"run"))
	{
		char* prog_start = find_prog(ramdisk,"program.img");
		exec_in_EL0(prog_start);
	}
	else if(!strcmp(command,"async_io"))
	{
		enable_interrupt();
		mini_uart_enable();
		send_async_string("this is async_io\r\n");
		core_timer_enable();
		void (*func)();
		func = recv_async_string;
		sleep(func,6);
		func = core_timer_disable;
		sleep(func,12);											//time will end in (start time + 12) second later
		func = disable_interrupt;
		sleep(func,12);
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
	else if(!strcmp(command,"p_alloc"))
	{			
		uart_send_string("KB you want to allocate : ");
		char c;
		int sum = 0;
		while((c = uart_recv()) != '\n')
		{
			sum *= 10;
			sum += (c-'0');
			uart_send(c);
		}
		uart_send_string("\r\n");
		page_alloc(sum);
	}
	else if(!strcmp(command,"p_free"))
	{
		uart_send_string("index you want to free : ");
		char c;
		int sum = 0;
		while((c = uart_recv()) != '\n')
		{
			sum *= 10;
			sum += (c-'0');
			uart_send(c);
		}
		uart_send_string("\r\n");
		int addr = sum * 4096;
		page_free(addr);
	}
	else if(!strcmp(command,"p_show"))
	{
		show_page();
	}
	else if(!strcmp(command,"d_alloc"))
	{
		char c;
		uart_send_string("size you want to alloc : ");
		int sum = 0;
		while((c = uart_recv()) != '\n')
		{
			sum *= 10;
			sum += (c-'0');
			uart_send(c);
		}
		uart_send_string("\r\n");
		char* addr = d_alloc(sum);
		uart_send_string("start address : ");
		uart_hex(addr);
		uart_send_string("\r\n");
	}
	else if(!strcmp(command,"clear_pool"))
	{
		clear_pool();
	}
	else if(!strcmp(command,"show_pool"))
	{
		show_pool_info();
	}
	else
	{
		uart_send_string("unvalid command! try <help>\r\n");
	}
	return;
}
