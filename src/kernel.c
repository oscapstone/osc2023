#include "mini_uart.h"
#include "string.h"
#include "mailbox.h"
#include "reboot.h"

#define BUFF_SIZE 100

char readBuf[BUFF_SIZE];

void shell(char* cmd){
	if(!strcmp(cmd, "help")){
		uart_send_string("help\t: print help menu.\nhello\t: print \"Hello World!\".\ninfo\t: print the hardware's information.\nreboot\t: reboot the device.\n");
	}else if(!strcmp(cmd, "hello")){
		uart_send_string("Hello World!\n");
	}else if(!strcmp(cmd, "info")){
		uart_send_string("Hardware's infomation:\n");
		get_board_revision();
		get_arm_memory();
	}else if(!strcmp(cmd, "reboot")){
		uart_send_string("Reboot the device.\n");
		reset(20);
	}else{
		uart_send_string("Command not found: ");
		uart_send_string(cmd);
		uart_send_string("\n");
	}
}

void kernel_main(void)
{	
	int count = 0;
	uart_init();
	uart_send_string("Hello!!!\r\n# ");

	while (1) {
		char recvChar = uart_recv();
		if(recvChar == '\r'){
			uart_send('\n');
			readBuf[count] = '\0';
			shell(readBuf);
			count = 0;
			readBuf[0] = '\0';
			uart_send_string("# ");
		}
		else{
			uart_send(recvChar);
			readBuf[count] = recvChar;
			count += 1;
		}
	}
}