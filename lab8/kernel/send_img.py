#!/usr/bin/env python3

import os
import time
import serial
import argparse


def main():
    parser = argparse.ArgumentParser(description='send kernel img')
    parser.add_argument('-i', '--img', type=str, default='kernel8.img', help='img filename')
    parser.add_argument('-d', '--device', type=str, default='/dev/ttyUSB0', help='device name')
    parser.add_argument('-b', '--baud', type=int, default=115200, help='baud rate')
    args, _ = parser.parse_known_args()

    imgsize = os.path.getsize(args.img)

    with open(args.img, 'rb') as f:
        with serial.Serial(args.device, args.baud) as tty:
            input("Press ENTER to send the image size: ")

            print(f'{args.img} is {imgsize} bytes and is sending now...')
            tty.write(imgsize.to_bytes(4, 'big'))

            input("Press ENTER to send the kernel image file: ")

            for i in range(imgsize):
                tty.write(f.read(1))
                tty.flush()
                time.sleep(0.001)


if __name__ == '__main__':
    main()