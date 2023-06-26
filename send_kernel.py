# import sys
# with open('kernel8.img', 'rb') as f:
#     kernel = f.read()


# path = '/dev/ttyUSB0'

# if len(sys.argv) > 1:
#     path = sys.argv[1]



# print(hex(len(kernel)))
# with open(path, 'wb') as tty:
#     kernel_len = len(kernel)
#     tty.write(b'S')
#     tty.write(bytes([(kernel_len & 0xff000000) >> 24]))
#     tty.write(bytes([(kernel_len & 0x00ff0000) >> 16]))
#     tty.write(bytes([(kernel_len & 0x0000ff00) >> 8]))
#     tty.write(bytes([kernel_len & 0x000000ff]))
#     tty.write(kernel)

import serial
import sys
with open('kernel8.img', 'rb') as f:
    kernel = f.read()


path = '/dev/ttyUSB0'

if len(sys.argv) > 1:
    path = sys.argv[1]
ser = serial.Serial(path, baudrate=115200)

if ser.is_open:
    kernel_len = len(kernel)
    ser.write(b'S')
    ser.write(bytes([(kernel_len & 0xff000000) >> 24]))
    ser.write(bytes([(kernel_len & 0x00ff0000) >> 16]))
    ser.write(bytes([(kernel_len & 0x0000ff00) >> 8]))
    ser.write(bytes([kernel_len & 0x000000ff]))
    ser.write(kernel)