#include <limits.h>
#include <stdbool.h>
#include <stdnoreturn.h>

#include "oscos/bootloader_crc32.h"
#include "oscos/delay.h"
#include "oscos/serial.h"

#define LEN_BYTES 4
#define CHECKSUM_BYTES 4
#define EXEC_KERNEL_DELAY_SEC (5 * NS_PER_SEC)

// Symbols defined in the linker script.
extern char _skernel[], _max_ekernel[];

/// \brief Loads the kernel from UART into the memory.
/// \return Whether or not the loading is successful.
static bool load_kernel(void) {
  // Read the length.

  size_t len_kernel = 0;
  for (int i = 0; i < LEN_BYTES; i++) {
    len_kernel = (len_kernel << CHAR_BIT) | (unsigned char)serial_getc();
  }

  // Check the length.

  const size_t max_len_kernel = _max_ekernel - _skernel;
  if (len_kernel <= max_len_kernel) {
    serial_putc('\x00');
  } else {
    serial_putc('\x01');
    return false;
  }

  // Read the kernel and calculate the checksum thereof.

  crc32_t crc32;
  crc32_init(&crc32);

  char *kernel_write_ptr = _skernel;
  for (size_t i = 0; i < len_kernel; i++) {
    const char byte = serial_getc();
    *kernel_write_ptr++ = byte;
    crc32_feed_byte(&crc32, byte);
  }

  // Read the checksum.

  uint32_t checksum = 0;
  for (int i = 0; i < CHECKSUM_BYTES; i++) {
    checksum = (checksum << CHAR_BIT) | (unsigned char)serial_getc();
  }

  // Verify the checksum.

  if (crc32_check(&crc32, checksum)) {
    serial_putc('\x00');
  } else {
    serial_putc('\x01');
    return false;
  }

  return true;
}

/// \brief Executes the loaded kernel.
///
/// Before calling this function, the kernel must have been properly loaded.
static noreturn void exec_kernel(void) {
  serial_deinit();

  // We don't need to synchronize the data cache and the instruction cache since
  // we haven't enabled any of them. Nonetheless, we still have to synchronize
  // the fetched instruction stream using the `isb` instruction.
  __asm__ __volatile__("isb\n"
                       "b _skernel");
  __builtin_unreachable();
}

noreturn void main(void) {
  serial_init();

  serial_set_mode(SM_BINARY);

  while (!load_kernel())
    ;
  delay_ns(EXEC_KERNEL_DELAY_SEC);
  exec_kernel();
}
