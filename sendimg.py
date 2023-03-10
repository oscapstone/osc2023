import argparse
import serial
import os
import sys
import numpy as np
import time

parser = argparse.ArgumentParser()
parser.add_argument("image")
parser.add_argument("tty")
args = parser.parse_args()

try:
    ser = serial.Serial(args.tty, 115200)
except:
    print("Serial init failed!")
    exit(1)

# usage
# $ python sendimg.py kernel8.img /dev/cu.usbserial-0001


def checksum(bytecodes):
    # convert bytes to int
    return int(np.array(list(bytecodes), dtype=np.int32).sum())

def receiveMsg():
    if ser.inWaiting() > 0:
        # 读取接收缓冲区中的所有数据
        data = ser.read(ser.inWaiting())
        print(data.decode())
    else:
        print('====No data available====')

def sendMsg():
    cmd = input("# ")
    if cmd == "exit":
        exit()
    cmd = '\n' + cmd + '\n'
    for c in cmd:
        ser.write(c.encode())
        time.sleep(0.01)
    # ser.write(b'\n')
    time.sleep(0.1)
    return 1

def main():
    file_path = args.image
    file_size = os.stat(file_path).st_size
    print(file_size)

    with open(file_path, 'rb') as f:
        bytecodes = f.read()

    file_checksum = checksum(bytecodes)

    # send command to Raspi 3
    time.sleep(0.1)
    cmd = "loadimg\n"
    for c in cmd:
        ser.write(c.encode())
        time.sleep(0.01)
    time.sleep(0.1)

    # Write file size to Raspi 3
    ser.write(file_size.to_bytes(4, byteorder="big"))
    time.sleep(0.01)

    # Show the message printed by Raspi 3
    receiveMsg()

    # # To communicate with Raspi 3 shell
    # while True:
    #     receiveMsg()
    #     if sendMsg():
    #         print("Write int into rapi 3")
    #         receiveMsg()
    #         time.sleep(0.01)
    #         ser.write(file_size.to_bytes(4, byteorder="big"))
    #     else:
    #         break


    # # Write checksum to Raspi 3
    # ser.write(file_checksum.to_bytes(4, byteorder="big"))


    per_chunk = 128
    # Compute the chunk count of image file
    chunk_count = file_size // per_chunk
    chunk_count = chunk_count + 1 if file_size % per_chunk else chunk_count

    for i in range(chunk_count):
        sys.stdout.write('\r')
        sys.stdout.write("%d/%d" % (i + 1, chunk_count))
        sys.stdout.flush()
        ser.write(bytecodes[i * per_chunk: (i+1) * per_chunk])
        while not ser.writable():
            pass
    receiveMsg()
    print("------------------")
    print(f"Image Size: {hex(file_size)}, Checksum: {file_checksum}")


if __name__ == "__main__":
    main()
