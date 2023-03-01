#include "str.h"
#include "terminal.h"
#include "uart.h"

int main(void) {
  uart_setup();
  terminal_run();

  return 0;
}
