import time
import os
import sys

BUD_RATE = 115200
FILENAME = '../kernel/kernel8.img'


def get_file_size(filename):
    return os.stat(filename).st_size


def write_by_block(orig_file, dest_file, block_size=1024, sleep_time=1):
    while (block := orig_file.read(block_size)):
        dest_file.write(block)
        time.sleep(sleep_time)


def send_by_block(orig_file, dest_file):
    """
    This is the max speed (bud rate / 10) of sending data to the UART in theory.
    When the received file broken, try reduce the block size.
    """

    size = get_file_size(FILENAME)
    dest_file.write(size.to_bytes(4, "big"))

    write_by_block(orig_file, dest_file, int(BUD_RATE / 10), 1)


def send_by_byte(orig_file, dest_file):
    """
    If send a large file by byte, it may stuck and cause the file broken.
    """

    raw = orig_file.read()

    size = len(raw)
    dest_file.write(size.to_bytes(4, "big"))

    for i in range(size):
        dest_file.write(raw[i:i+1])


def main():
    with open(sys.argv[1], 'wb', buffering=0) as tty:
        with open(FILENAME, 'rb') as file:
            # send by block by default
            send_by_block(file, tty)


if __name__ == '__main__':
    main()
