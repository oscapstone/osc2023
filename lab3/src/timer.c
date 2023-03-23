#include "timer.h"
#include "uart.h"

#define TIMER_LIMIT 10

// The global timer queue
timer_Q queue[TIMER_LIMIT];

int add_timer(int (*fn)(void*), int second, void* arg){
	for(int i = 0; i < TIMER_LIMIT; i++){
		if(!queue[i].used){
			queue[i].used = 1;
			queue[i].fn = fn;
			queue[i].remain = second;
			queue[i].arg = arg;
			return 0;
		}
	}
	return 1;
}

static int warp_uart_puts(void* s){
	uart_puts((char*) s);
	uart_puts("\n");
	return 0;
}

static int warp_uart_a_gets(void* s) {
	char tmp[100] = {0};
	uart_a_gets(tmp, 100);
	uart_puts("Async get: ");
	uart_puts(tmp);
	uart_puts("\n");
	return 0;
}

int set_timeout(char* message, int second){
	add_timer(warp_uart_puts, second, (void*)message);
	return 0;
}

int set_timer_read(char* message){
	add_timer(warp_uart_a_gets, 8, 0);
	return 0;
}

int init_timer_Q(void){
	for(int i = 0; i < TIMER_LIMIT; i++){
		queue[i].used = 0;
	}
	return 0;
}

int timer_walk(int sec){
	for(int i = 0; i < TIMER_LIMIT; i++){
		if(!queue[i].used)
			continue;
		queue[i].remain -= sec;
		if(queue[i].remain <= 0){
			queue[i].fn(queue[i].arg);
			queue[i].used = 0;
		}
	}
	return 0;
}
			
	
