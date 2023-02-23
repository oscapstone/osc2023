#include "terminal.h"
#include "str.h"
#include "uart.h"
#include "mailbox.h"

#define BUF_LEN 256

void terminal_run(){
	char input_buf[BUF_LEN + 1];
	char* tmp;

	while(1){
		tmp = input_buf;
		for(int i = 0; i < BUF_LEN; i++){
			*tmp = uart_getc();
			uart_send(*tmp);

			if(*tmp == 127){
				*tmp = 0;
				tmp --;
				*tmp = 0;
				tmp --;
				uart_send('\b');
				uart_send(' ');
				uart_send('\b');
			}
			if(*tmp == '\n'){
				*(tmp) = '\0';
				break;
			}
			tmp ++;
		}

		if(!strcmp(input_buf, "help"))
			help();
		else if(!strcmp(input_buf, "hello"))
			hello();
		else if(!strcmp(input_buf, "lshw"))
			lshw();
		//else if(!strcmp(input_buf, "reboot"))
			//reboot();
		else
			invalid_command(input_buf);
	}
}
			
int help(){
	uart_puts(	
			"help\t: print this help message.\n"
			"hello\t: print Hello World!\n"
			"lshw\t: print the information of device \n"
			"reboot\t: reboot the device.\n"
		 );
	return 0;
}

int hello(){
	uart_puts("Hello World!\n");
	return 0;
}

int invalid_command(const char* s){
	uart_puts(s);
	uart_puts("Invalid command! Please use `help` to list commands\n");
	return 0;
}

int lshw(void){
	uart_puts("Board version\t: ");
	mbox[0] = 7 * 4;
	mbox[1] = MAILBOX_REQ;
	mbox[2] = TAG_BOARD_VER;
	mbox[3] = 4;
	mbox[4] = 0;
	mbox[5] = 0;
	mbox[6] = TAG_LAST;

	if(mailbox_config(CHANNEL_PT)){
		uart_puth(mbox[5]);
	}
	uart_puts("\nDevice base Mem Addr\t: ");
	mbox[0] = 8 * 4;
	mbox[1] = MAILBOX_REQ;
	mbox[2] = TAG_ARM_MEM;
	mbox[3] = 8;
	mbox[4] = 0;
	mbox[5] = 0;
	mbox[6] = 0;
	mbox[7] = TAG_LAST;
	if(mailbox_config(CHANNEL_PT)){
		uart_puth(mbox[5]);
		uart_puts("\nDevice Mem size\t: ");
		uart_puth(mbox[6]);
	}
	uart_putc('\n');
	return 0;
}
	
	
