#include "uart.h"

#define BOOT_LOADER_ADDR 0x60000
#define KERNEL_ADDR 0x80000
extern unsigned long __bss_end;

void main(){
  register unsigned long x0 asm("x0");
  unsigned long DTB_BASE = x0;
  uart_init();
  char* ptr_bootloader = (char*)BOOT_LOADER_ADDR;
  char* ptr_kernel = (char*)KERNEL_ADDR;
  char* ptr_bssend = (char*)(&__bss_end);

  while(ptr_kernel != ptr_bssend){
    *ptr_bootloader = *ptr_kernel;
    ptr_bootloader ++;
    ptr_kernel ++;
  }
  /*
  0x20000 - 4 = 131068
  the label uased in branch instruction is pc-relative
  */ 
  asm volatile("b -131068");
  while(1){
    char t = uart_getc();
    if(t == 'c'){
      break;
    }
  }
  int kernel_size = uart_get_int();
  uart_send_int(kernel_size);
  char *kernel = (char *)0x80000;
  for(int i = 0; i < kernel_size; i++){
    char c = uart_recv();
    // uart_send_int(c);
    char content = (c & 0xFF);
    *(kernel + i) = content;
  }
  while (1){
    char t = uart_getc();
    if(t == 'c'){
      uart_puts("UBOOT\r\n");
      break;
    }
  }
  while(1){
    char t = uart_getc();
    if(t == 's'){
      break;
    }
  }
  void (*kernel_boot)(unsigned long) = (void *)kernel;
  kernel_boot(DTB_BASE);
}