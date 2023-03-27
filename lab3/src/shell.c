#include "mini_uart.h"
#include "string.h"
#include "mbox.h"
#include "reboot.h"
#include "ramdisk.h"
#include "to_EL0.h"
#include "check_interrupt.h"
#include "timer.h"

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
		send_async_string("this is async_ioop\r\n");
		recv_async_string();
		disable_interrupt();
	}
	else if(!strcmp(command,"sleep"))
	{
		enable_interrupt();
		extern void core_timer_enable();
		extern void core_timer_disable();
		core_timer_enable();
		init_t_queue();
		setTimeout("this will print after 6 second\r\n",6);
		setTimeout("this will print after 4 second\r\n",4);
		core_timer_disable();
		disable_interrupt();
	}
	else
	{
		uart_send_string("unvalid command! try <help>\r\n");
	}
	return;
}
