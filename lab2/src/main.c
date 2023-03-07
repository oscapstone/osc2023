#include "terminal.h"
#include "uart.h"
#include <string.h>

int main(void) {
  uart_setup();
  terminal_run();

  return 0;
}
