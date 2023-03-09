#include "terminal.h"
#include "uart.h"
#include "heap.h"
#include <string.h>

int main(void) {
  uart_setup();
  heap_init();
  terminal_run();

  return 0;
}
