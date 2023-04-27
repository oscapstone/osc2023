import argparse
import os
import time
import math
import serial

parser = argparse.ArgumentParser(description='*.img uart sender')
parser.add_argument('-i', '--img', default='kernel8.img', type=str)
parser.add_argument('-d', '--device', default='/dev/ttyUSB0', type=str)
parser.add_argument('-b', '--baud', default=115200, type=int)

args = parser.parse_args()

img_size = os.path.getsize(args.img)

with open(args.img, 'rb') as f:
    with serial.Serial(args.device, args.baud) as tty:
        
        print(f'{args.img} is {img_size} bytes')
        print('img file is now sending')

        tty.write(img_size.to_bytes(4, 'big'))

        input()

        for i in range(img_size):
            tty.write(f.read(1))
            tty.flush()
            time.sleep(0.001)

        print('img sent')
