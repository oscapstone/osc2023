import serial
import sys
import time

# ser = serial.Serial('/dev/cu.usbserial-0001', 115200)

try:
    filename = sys.argv[1]
except(IndexError):
    print("Need filename!")
    exit(1)

try:
    with open('/dev/cu.usbserial-0001', "wb", buffering = 0) as tty:
        tty.write(str.encode("hello\n"));
    # with open(filename) as f:
    #     for line in f:
    #         ser.write(str.encode(line))
    #         time.sleep(0.1)
except(FileNotFoundError):
    print("File Not Found!")
    exit(1)
