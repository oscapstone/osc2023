#include "mini_uart.h"
#define MAX_CMD 64
#define KSIZE 5

void kernel_main(void)
{
	uart_init();
	
	// char *kernel=(char*)0x80000;
	volatile unsigned char *kernel = (unsigned char *)0x80000;
	char * size_ch[KSIZE];
	int size = 0;
again:
	size = 0;
	// uart_puts("Kernel Size (4 digits):");
	
	char tp;
	// while ((tp = uart_getc()) != '\n')
    // {
	// 	uart_send(tp);
	// 	size*=10;
	// 	size+=(tp-'0');
    // }
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
	// size = 2606;

	void *memory = (void *)0x08200000;
	for (int i = 0; i < 100; i++) {
        uart_hexdump((*(char*)(memory+i)));
		uart_send(' ');
    }

	if (size < 64 || size > 10000){
		uart_send('E');
		uart_send('L');
		goto again;
	}
	uart_puts("\n");

	// if(uart_getc() == '\n'){
		// uart_puts("\nTransmission Start:\n");
	// }
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
		// uart_int(size);
		uart_send(' ');
		kernel++;
	}


	uart_puts("Finish\n");
	// char *kernel=(char*)0x80000;
	// uart_hexdump(*kernel);
	// char *kernel_debug=(char*)0x80000;
	// for(int i = 0; i < 100; i++){
	// 	uart_hexdump(*kernel_debug+i);
	// }
	for (int i = 0; i < 100; i++) {
        uart_hexdump((*(char*)(memory+i)));
		uart_send(' ');
    }
	
	asm volatile (
        // we must force an absolute address to branch to
        "mov x30, #0x80000; mov x0, x28; ret"
    );
	uart_puts("Failed to load, no return\n");
	return;
}
