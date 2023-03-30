#include "mini_uart.h"
#include "string.h"
#include "print.h"
#include "type.h"
#include "read.h"

#define BUF_SIZE 64
#define KERNEL_ADDRESS 0x80000

void bootloader_main() {
  uart_init();
  print("Welcome to Bootloader!!\n");
  print("Please send kernel image from UART...\n");

  char buf[BUF_SIZE];

  while (1) {
    if (readline(buf, BUF_SIZE, false) >= 0 || streq(buf, "kernel_image") == 0) {
      break;
    }
  }

  printf("Bootloader recieves the identifier: %s\n", buf);

  readline(buf, BUF_SIZE, false);
  int kernel_size = strtoi(buf, 10);
  printf("Bootloader recieves the kernel image's size: %d\n", kernel_size);

  char ch;
  char *kernel_addr = (char *) KERNEL_ADDRESS;
  for (int i = 0; i < kernel_size; i++) {
    ch = uart_recv();
    uart_send_int(ch);
    *(kernel_addr + i) = ch;
  }
  print("received the kernel image and ready to branch\n");

  while (1) {
    asm volatile("nop");
    ch = uart_recv();
    if (ch == 'j') {
      printf("%c", ch);
      break;
    }
  }

  void (*jump_to_kernel)() = (void *)KERNEL_ADDRESS;
  jump_to_kernel();

  print("should not print this message\n");
}
