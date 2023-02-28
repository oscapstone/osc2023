#include "oscos/serial.h"

void main() {
  serial_init();

  serial_lock();

  serial_puts("UART Test");
  for (;;) {
    serial_putc(serial_getc());
  }
}
