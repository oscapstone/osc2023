import serial
import os
import time
import argparse

def sendKernelImg(s, f):
    tty = serial.Serial(s, 115200, timeout=0.5)
    file_stats = os.stat(f)
    # Send Boot cmd
    tty.write('Boot\n'.encode('utf-8')) # End by
    # Send kernel size
    tty.write(str(file_stats.st_size).encode('utf-8'))
    tty.write('\n'.encode('utf-8')) # End by \n
    # Send kernel image byte-by-byte
    with open(f, "rb") as fp:
        byte = fp.read(1)
        while byte:
            tty.write(byte)
            byte = fp.read(1)
            time.sleep(0.0001)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", help="enter tty", required=True)
    parser.add_argument("-f", help="enter img", required=True)
    args = parser.parse_args()
    sendKernelImg(args.s, args.f)
