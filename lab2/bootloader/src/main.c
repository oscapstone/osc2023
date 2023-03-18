#include "mini_uart.h"
#define MAX_CMD 64
#define KSIZE 5

void kernel_main(void)
{
	uart_init();
	
	volatile unsigned char *kernel = (unsigned char *)0x80000;
	char * size_ch[KSIZE];
	int size = 0;
again:
	size = 0;
	
	char tp;
	for(int i = 0; i < 4; i++){
		
		tp = uart_getc();
		if(tp < '0' || tp > '9'){
			i--;
			continue;
		}
		uart_send(tp);
		size*=10;
		size+=(tp-'0');
	}
	uart_puts("\n");
	uart_int(size);
	uart_puts("\nStart: ");

	if (size < 64 || size > 10000){
		uart_send('E');
		uart_send('L');
		goto again;
	}
	uart_puts("\n");

	int cnt = 0;
	char c;
	while (size--){
		c = uart_getc();
		
		if(cnt% 16 == 0){
			uart_puts("\n");
			uart_hex(kernel);
			uart_send(' ');
		}
		cnt += 1;
		*kernel=c;
		uart_hexdump(c);
		uart_send(' ');
		kernel++;
	}


	uart_puts("Finish\n");

	asm volatile (
        // we must force an absolute address to branch to
        "mov x30, #0x80000; mov x0, x28; ret"
    );
	uart_puts("Failed to load, no return\n");
	return;
}
