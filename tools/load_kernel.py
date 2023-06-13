import sys
import argparse
import struct
import time

# Protocol: send the length of kernel as an unsigned integer with little endian. 
# And then send all of kernel

def send_kernel_rpi3(args):
    with open(args.file, 'rb') as kernel_file:
        kernel = kernel_file.read()
        print(len(kernel))

    with open(args.dev, 'wb' , buffering=0) as target:
        time.sleep(1)
        target.write(struct.pack('<I', len(kernel)))
        time.sleep(1)
        target.write(kernel)

def send_kernel_qemu(args):
    
    with open(args.file, 'rb') as kernel_file:
        kernel = kernel_file.read()
        print(len(kernel))

    with open(args.dev, 'wb', buffering=0) as target:
        target.write(struct.pack('<I', len(kernel)))
        target.write(kernel)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dev',help="The device path ready for connection")
    parser.add_argument('--file',help="The file path ready for writing")
    args = parser.parse_args()

    if 'ttyUSB0' in args.dev:
        send_kernel_rpi3(args)
    else:
        send_kernel_qemu(args)
