#include "oscos/serial.h"
#include "oscos/shell.h"

void main() {
  serial_init();

  run_shell();
}
