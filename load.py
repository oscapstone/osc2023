#!python3
import os
import sys

if len(sys.argv) < 2:
    print('usage: ./load.py <tty-path> [kernel-path]')
    exit()

tty = sys.argv[1]
kernel = 'kernel8.img' if len(sys.argv) < 3 else sys.argv[2]

size = os.path.getsize(kernel)
print('kernel size: ', size)

tty = open(tty, 'wb', buffering=0)
kernel = open(kernel, 'rb')

tty.write(size.to_bytes(4, 'little'))
while byte := kernel.read(1):
    if byte == b'\r':
        tty.write(b'\r')
    tty.write(byte)

tty.close()
kernel.close()
