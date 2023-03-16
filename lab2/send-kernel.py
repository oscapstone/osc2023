#!/usr/bin/env python3

import os
import struct
import sys

import serial


class CRC32:
    # Note that this implementation of CRC32 is meant to be used solely for
    # verifying the kernel in the bootloader and does not aim to conform to any
    # existing standards. As long as the sender and the bootloader agree with
    # the CRC32 implementation, everything should be fine.

    POLY = 0x04c11db7

    def __init__(self) -> None:
        self._crc = 0

    def feed_bit(self, x: bool) -> None:
        self._crc = (((self._crc & 0x7fffffff) << 1) | x) \
            ^ (CRC32.POLY if self._crc >> 31 else 0)

    def feed_byte(self, x: int) -> None:
        if not 0 <= x < 256:
            raise ValueError('Value must be representable by one byte')

        for _ in range(8):
            self.feed_bit(x & 0x80 != 0)
            x <<= 1

    def feed_bytes(self, xs: bytes) -> None:
        for x in xs:
            self.feed_byte(x)

    def finalize(self) -> int:
        for _ in range(4):
            self.feed_byte(0)

        return self._crc


SERIAL_BAUDRATE = 115200
BOOTLOADER_TIMEOUT_SEC = 10
XFER_CHUNK_SZ = 4096


def bootloader_timeout() -> None:
    print('Error: No response from the bootloader within '
          + f'{BOOTLOADER_TIMEOUT_SEC} seconds', file=sys.stderr)
    exit(1)


def main():
    _, kernel_path, serial_path = sys.argv

    with (serial.Serial(serial_path, SERIAL_BAUDRATE,
                        timeout=BOOTLOADER_TIMEOUT_SEC) as tty,
          open(kernel_path, 'rb') as kernel):
        len_kernel = os.fstat(kernel.fileno()).st_size

        # Send the length of the kernel.

        tty.write(struct.pack('!I', len_kernel))

        response = tty.read(1)
        if len(response) != 1:
            bootloader_timeout()

        if response[0] != 0:
            print('Error: The kernel is too big', file=sys.stderr)
            exit(1)

        # Send the kernel and calculate the checksum thereof.

        crc32 = CRC32()

        while True:
            chunk = kernel.read(XFER_CHUNK_SZ)
            if len(chunk) == 0:
                break
            n_bytes_written = tty.write(chunk)
            assert n_bytes_written == len(chunk)

            crc32.feed_bytes(chunk)

        # Verify the checksum.

        checksum = crc32.finalize()
        tty.write(struct.pack('!I', checksum))

        response = tty.read(1)
        if len(response) != 1:
            bootloader_timeout()

        if response[0] != 0:
            print('Error: The kernel is corrupted', file=sys.stderr)
            exit(1)


if __name__ == '__main__':
    main()
