import argparse
import serial
import os
import sys
import numpy as np

parser = argparse.ArgumentParser(description='NYCU OSC kernel sender')
parser.add_argument('--image', metavar='PATH', default='./kernel8.img', type=str, help='path to kernel8.img')
parser.add_argument('--device', metavar='TTY',default='/dev/ttyUSB0', type=str,  help='path to UART device')
parser.add_argument('--baud', metavar='Hz',default=115200, type=int,  help='baud rate')
args = parser.parse_args()


def checksum(bytecodes):
    # convert bytes to int
    return int(np.array(list(bytecodes), dtype=np.int32).sum())


def main():
    try:
        ser = serial.Serial(args.device, args.baud)
    except:
        print("Serial init failed!")
        exit(1)

    file_path = args.image
    file_size = os.stat(file_path).st_size

    with open(file_path, 'rb') as f:
        bytecodes = f.read()

    file_checksum = checksum(bytecodes)

    ser.write(file_size.to_bytes(4, byteorder="big"))
    ser.write(file_checksum.to_bytes(4, byteorder="big"))

    print(f"Image Size: {file_size}, Checksum: {file_checksum}")

    per_chunk = 1
    chunk_count = file_size // per_chunk
    chunk_count = chunk_count + 1 if file_size % per_chunk else chunk_count

    for i in range(chunk_count):
        sys.stdout.write('\r')
        sys.stdout.write("%d/%d" % (i + 1, chunk_count))
        sys.stdout.flush()
        ser.write(bytecodes[i * per_chunk: (i+1) * per_chunk])
        while not ser.writable():
            pass


if __name__ == "__main__":
    main()