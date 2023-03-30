#!/usr/bin/env python3

import os
import time
import serial

class UARTInteractor:
    def __init__(self, device_name):
        self.serial = serial.Serial(device_name, 115200)

    def send_int(self, number):
        if number > 2 ** 32 - 1:
            raise 'Number can only be 4 bytes long'
        number_in_bytes = number.to_bytes(4, byteorder='big')
        return self.send_bytes(number_in_bytes)

    def write_string(self, string):
        if not string.endswith('\n'):
            string += '\n'
        return self.serial.write(string.encode('ascii'))

    def write_bytes(self, byte):
        return self.serial.write(byte)

    def read(self, max_len):
        return self.serial.read(max_len)

    def read_int(self):
        bytes_to_read = 4
        number_bytes = self.read(bytes_to_read)
        return int.from_bytes(number_bytes, byteorder='big')

    def read_buf(self):
        return self.serial.read(self.serial.in_waiting)
    
    def read_buf_string(self):
        return self._decode(self.read_buf())
    
    def read_line(self):
        return self._decode(self.serial.readline())

    def _decode(self, byte):
    #     print(byte)
        return byte.decode('ascii')

def main():

    tmp = input('Please enter the device name (default is /dev/ttys001): ')
    if tmp != '':
        device_name = tmp
    else:
        device_name = '/dev/ttys001'

    uart_inter = UARTInteractor(device_name)

    uart_inter.write_string('kernel_image')
    print("send the identifier: kernel_image")
    time.sleep(0.001)
    print(uart_inter.read_line())

    tmp = input('Please enter the filename of your kernel image (default is kernel8.img): ')
    if tmp != '':
        kernel_image = tmp
    else:
        kernel_image = 'kernel8.img'

    kernel_size = os.path.getsize(kernel_image)
    print(f"kernel image's size: {kernel_size}. Send the size to the bootloader...")

    uart_inter.write_string(str(kernel_size))
    time.sleep(0.001)
    print(uart_inter.read_line())

    print("Do you want to send the kernel image [Y/n]")

    with open(kernel_image, 'rb') as k:
        kernel = k.read()
        LINE_UP = '\033[1A'
        LINE_CLEAR = '\x1b[2K'  
        total = len(kernel)
        cur = 1
        for i in kernel:
            content = i.to_bytes(1, byteorder='big')
            uart_inter.write_bytes(content)
            print(f"PROGRESS: {cur}/{total} ", i, " ", uart_inter.read_int())
            time.sleep(0.001)
            if cur != total:
                print(LINE_UP, end=LINE_CLEAR)
            cur += 1
    print(uart_inter.read_line())

if __name__ == '__main__':
    main()
