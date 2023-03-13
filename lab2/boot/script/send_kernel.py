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

img_size_bytes = img_size.to_bytes(4, 'big')

for i in range(4):
        print(img_size_bytes[i].to_bytes(1, 'big'))
        tty.write(img_size_bytes[i].to_bytes(1, 'big'))
        tty.flush()
        sleep(0.001)

with open(args.img, 'rb') as f:
        for i in range(img_size):
                k = f.read(1)
                tty.write(k)
                tty.flush()
                sleep(0.001)

print('Finished sending kernel')