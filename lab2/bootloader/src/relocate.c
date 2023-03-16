extern unsigned char _loader_begin, _loader_end, __relocate_point, main;
#include "uart.h"
#include "relocate.h"

void int_to_str(int n, char *s){
  char tmp[100];
  int idx = 0;
  do{
    //'0':48
    tmp[idx] = (char)((n%10) + 48); //idx 0->++, units->tens->hundreds
    idx++; //string end
    n /= 10;
  } while(n > 0);
  for (int i=0; i<idx; i++){  //s[0] -> ++, hundreds->tens->units
    s[i] = tmp[idx-i-1];
  }
  s[idx] = '\0'; //string end 
}

void relocate() {
	unsigned long loader_size = (&_loader_end - &_loader_begin);
	unsigned char *new_bl = (unsigned char*)&__relocate_point; //move to 0x60000
	unsigned char *bl = (unsigned char*)&_loader_begin;

	while (loader_size --){
		*new_bl++ = *bl;
		bl++;
		//*bl++ = 0; //clear

	}

	void (*func_p)(void) = &main; 
	func_p -= 0x20000; //go to 0x60000 and do.
	func_p();
}


void loadimg() {
	long long address = KERNEL_START; //0x80000 
    uart_send_string("Send image via UART now!\n");
    uart_flush();
    char p[20];

    // big endian
    int img_size = 0, i;
    for (i = 0; i < 4; i++) {  
        img_size <<= 8;
        img_size |= (int)uart_read_raw();
    }
    uart_send_string("Image size : ");
    int_to_str(img_size, p);
    uart_send_string(p);
    uart_send_string("\n");

    // big endian
    int img_checksum = 0;
    for (i = 0; i < 4; i++) {
        img_checksum <<= 8;
        img_checksum |= (int)uart_read_raw();
    }
    uart_send_string("Check sum : ");
    int_to_str(img_checksum, p);
    uart_send_string(p);
    uart_send_string("\n");

    char *kernel = (char *)address;

    for (i = 0; i < img_size; i++) {
        char b = uart_read_raw();
        *(kernel + i) = b;
        img_checksum -= (int)b;
        /* for debug
        int_to_str(i, ii);
        uart_send_string(ii);
        uart_send_string("\n");
        */

    }

    if (img_checksum != 0) {
        uart_send_string("Failed!");
        uart_send_string("\n");
        int_to_str(img_checksum, p);
        uart_send_string(p);
        uart_send_string("\n");
    }
    else {
        uart_send_string("Done");
        uart_send_string("\n\n");
        register unsigned int r;
        r = 1000;
        while(r--) { asm volatile("nop"); }

        void (*start_os)(void) = (void *)KERNEL_START; //do 0x80000 kernel start.
        start_os();
    }	
}