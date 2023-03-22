#include "oscos/bcm2837/mailbox_info.h"
#include "oscos/devicetree.h"
#include "oscos/serial.h"
#include "oscos/shell.h"

void main(const void *const dtb_start) {
  devicetree_register(dtb_start);

  serial_init();

  const uint32_t board_revision = get_board_revision();
  const arm_memory_t arm_memory = get_arm_memory();

  serial_fputs("Board revision: 0x");
  serial_print_hex(board_revision);
  serial_fputs(", ARM memory: Base: 0x");
  serial_print_hex(arm_memory.base);
  serial_fputs(", Size: 0x");
  serial_print_hex(arm_memory.size);
  serial_putc('\n');

  run_shell();
}
