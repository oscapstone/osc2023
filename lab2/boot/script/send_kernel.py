import serial
import os
import argparse
from time import sleep

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--img', type=str)
parser.add_argument('-p', '--port', type=str, default="/dev/ttyUSB0")
args = parser.parse_args()

try:
        img_size = os.path.getsize(args.img)
except:
        print(args.img, ": no such file")
        exit()

print("Kernel ", args.img, " size: ", img_size, "bytes")

tty = serial.Serial(args.port, 115200)

tty.write(img_size.to_bytes(4, 'big'))

with open(args.img, 'rb') as f:
        for i in range(img_size):
                k = f.read(1)
                tty.write(k)
                sleep(0.00005)

print('Finished sending kernel')