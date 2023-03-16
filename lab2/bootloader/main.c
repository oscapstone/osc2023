#include "mini_uart.h"
#include "string.h"
#include "print.h"
#include "type.h"
#include "read.h"

#define BUF_SIZE 64
#define KERNEL_ADDRESS 0x80000

extern void store_kernel_image(char, unsigned long);
extern void jump_to(unsigned long);


void bootloader_main() {
  uart_init();
  print("Welcome to Bootloader!!\n");
  print("Please send kernel image from UART...\n");
  
  char buf[BUF_SIZE];
  if (readline(buf, BUF_SIZE, false) <= 0 || streq(buf, "kernel_image") != 0) {
    return;
  }
  printf("kernel image's identifier: %s\n", buf);
  
  readline(buf, BUF_SIZE, false);
  int kernel_size = strtoi(buf, 10);
  printf("kernel image's size: %d\n", kernel_size);
  
  char ch;
  unsigned long addr = KERNEL_ADDRESS;
  for (int i = 0; i < kernel_size; i++) {
    ch = uart_recv();
    store_kernel_image(ch, addr);
    addr += 1;
  }
  print("received the kernel image and ready to branch\n");
  jump_to(KERNEL_ADDRESS);
  print("should not print this message\n");
}
