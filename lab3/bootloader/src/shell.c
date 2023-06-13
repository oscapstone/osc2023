#include "uart.h"
#include "utils.h"
#include "mbox.h"
#include "relocate.h"

void shell_init(){
	uart_init();
	uart_send_string("Hello! Welcome to Chely's system.\n That's demo Lab2.\n");
	uart_send_string("(bootloader)\n");
	uart_flush();
}


void shell_input(char *cmd){
	uart_send_string("\r> ");
	cmd[0] = '\0';
	int end =0; 
	char c;
	char s[2];

	while((c =uart_recv()) != '\n') {
		s[0]=c;
		s[1]='\0';
		uart_send_string(s);
		cmd[end] = c;
		end ++;
		cmd[end]= '\0';
	}
	uart_send_string("\n");
	
}


void shell_command(char *cmd){
	if(cmd[0] =='\0') return;
	if (str_compare(cmd, "hello")){
		uart_send_string("Hello World! \n");
	}
	else if (str_compare(cmd, "help")){
		uart_send_string("Command|Description\n");
		uart_send_string("-------|-----------------------------\n");
		uart_send_string("hello  |print Hello World!\n");
		uart_send_string("mailbox|print hardware information.\n");
		uart_send_string("help   | print all available commands\n");
		uart_send_string("reboot |reboot raspi\n");
		uart_send_string("boot   | boot from uart\n");
	}
	else if (str_compare(cmd, "reboot")){
		uart_send_string("rebooting ............zzzzzzzzzz\n");
		reset(3000);
		while(1); //prevent sth error.
	}
	else if (str_compare(cmd, "mailbox")){
		mbox_get_board_revision();
		mbox_get_arm_memory();
	}
	else if (str_compare(cmd, "boot")){
		loadimg();
	}

	else{
		uart_send_string(cmd);
		uart_send_string(": Command Not found!\n");
	}

}


int str_compare(char *s1, char *s2){
	int i=0;
	while(s1[i]){
		if (s1[i] != s2[i]){
			return 0;
			break;
		}
		i++;
	}
	return 1;
}


