#ifndef OSCOS_BOOTLOADER_CRC32_H
#define OSCOS_BOOTLOADER_CRC32_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint32_t crc;
} crc32_t;

void crc32_init(crc32_t *crc32);

void crc32_feed_byte(crc32_t *crc32, unsigned char x);

bool crc32_check(crc32_t *crc32, uint32_t checksum);

#endif
