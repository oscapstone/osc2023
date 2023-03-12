#!/usr/bin/env python3

import os
import serial

class UARTInteractor:
    def __init__(self, device_name):
        self.serial = serial.Serial(device_name)

    def write_string(self, string):
        if not string.endswith('\n'):
            string += '\n'
        return self.serial.write(string.encode('ascii'))

    def write_bytes(self, byte):
        return self.serial.write(byte)

    def read_string(self):
        return self._decode(self.serial.read())

    def _decode(self, byte):
        return byte.decode('ascii')

def main():

    tmp = input('Please enter the device name (default is /dev/ttys001): ')
    if tmp != '':
        device_name = tmp
    else:
        device_name = '/dev/ttys001'

    uart_inter = UARTInteractor(device_name)
    uart_inter.write_string('kernel_image')

    tmp = input('Please enter the filename of your kernel image (default is kernel8.img): ')
    if tmp != '':
        kernel_image = tmp
    else:
        kernel_image = 'kernel8.img'

    kernel_size = os.path.getsize(kernel_image)
    print(f"size of kernel image: {kernel_size}")
    uart_inter.write_string(str(kernel_size))

    print("Do you want to send the kernel image [Y/n]")

    with open(kernel_image, 'rb') as k:
        kernel = k.read()
        uart_inter.write_bytes(kernel)

if __name__ == '__main__':
    main()
