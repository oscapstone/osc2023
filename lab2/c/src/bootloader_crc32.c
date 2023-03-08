// Note that this implementation of CRC32 is meant to be used solely for
// verifying the kernel in the bootloader and does not aim to conform to any
// existing standards. As long as the sender and the bootloader agree with the
// CRC32 implementation, everything should be fine.

#include "oscos/bootloader_crc32.h"

#define POLY ((uint32_t)0x04c11db7)

void crc32_init(crc32_t *const crc32) { crc32->crc = 0; }

static void _crc32_feed_bit(crc32_t *const crc32, const bool x) {
  crc32->crc = ((crc32->crc << 1) | x) ^ (POLY & ((int32_t)crc32->crc >> 31));
}

void crc32_feed_byte(crc32_t *const crc32, unsigned char x) {
  for (int i = 0; i < 8; i++) {
    _crc32_feed_bit(crc32, x >> 7);
    x <<= 1;
  }
}

bool crc32_check(crc32_t *const crc32, uint32_t checksum) {
  for (int i = 0; i < 4; i++) {
    crc32_feed_byte(crc32, checksum >> 24);
    checksum <<= 8;
  }

  return crc32->crc == 0;
}
