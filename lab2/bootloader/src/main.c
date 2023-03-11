#include "mini_uart.h"
#define MAX_CMD 64

void kernel_main(void)
{
	uart_init();
	
	// char *kernel=(char*)0x80000;
	volatile unsigned char *kernel = (unsigned char *)0x80000;
	int size = 0;
again:
	// uart_puts("Kernel Size:");
	// size=uart_getc();
	// size|=uart_getc()<<8;
	// size|=uart_getc()<<16;
	// size|=uart_getc()<<24;
	size = 2402;
	// size = 2606;

	if (size < 64 || size > 1024*1024){
		uart_send('E');
		uart_send('L');
		goto again;
	}

	// if(uart_getc() == '\n'){
		uart_puts("Transmission Start:\n");
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
		kernel++;
	}
	// for (int i = 0; i < size; i++) {
    //     *(code + i) = (unsigned char)uart_getc_pure();
    // }

	uart_puts("Finish\n");
	// char *kernel=(char*)0x80000;
	// uart_hexdump(*kernel);
	// char *kernel_debug=(char*)0x80000;
	// for(int i = 0; i < 100; i++){
	// 	uart_hexdump(*kernel_debug+i);
	// }
	
	asm volatile (
        // we must force an absolute address to branch to
        "mov x30, #0x80000; mov     sp, 0x60000; ret"
    );
	uart_puts("Failed to load, no return\n");
	return;
}
