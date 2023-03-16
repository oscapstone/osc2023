#include "uart.h"
#include "utils.h"
#include "mbox.h"
#include "cpio.h"

void shell_init(){
	uart_init();
	uart_send_string("Hello! Welcome to Chely's system.\n That's demo Lab1.\n");
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
	//for sub cmd:
	char sub_cmd3[4];
	for (int i=0; i<3;i++) sub_cmd3[i] = cmd[i];
	sub_cmd3[3] = '\0';

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
	//implement ls 
	else if (str_compare(cmd, "ls")){
		list();
	}
	//implement cat file
	else if (str_compare(sub_cmd3, "cat")){
		char *cat_file_name = cmd+3;
		while(cat_file_name[0] == ' ') cat_file_name++;
		show_cat_file(cat_file_name);
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


int str_cmp(char *s1, char *s2){
  int i = 0;
  if (s1[0] == '\0' && s2[0] == '\0') return 1;
  while(s1[i]){
    if (s1[i] != s2[i]) return 0;
    i++;
  }

  if (s2[i] == '\0') return 1;
  return 0;
}
