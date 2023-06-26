#include <shell.h>

#include <uart.h>
#include <string.h>
#include <command.h>

void shell(){
	char buf[BUF_SIZE];
	while(1){
		uart_print("# ");
		uart_readline(buf);
		if(strncmp(buf, "help", 4) == 0){
			help();
		}else if(strncmp(buf, "hello", 5) == 0){
			hello();
		}else if(strncmp(buf, "reboot", 6) == 0){
			reboot();
		}else if(strncmp(buf, "ls", 2) == 0){
			ls();
		}else if(strncmp(buf, "cat", 2) == 0){
			uart_print("Filename: ");
			uart_readline(buf);
			cat(buf);
		}else if(strncmp(buf, "exec", 4) == 0){
			exec(&buf[5]);
		}else if(strncmp(buf, "setTimeout", 10) == 0){
			setTimeout();
		}else{
			uart_print("Unknown command: [");
			uart_print(buf);
			uart_print("].");
			newline();
		}
	}
}
