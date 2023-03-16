#!/usr/bin/env python3

from serial import Serial  # Import Serial class from pyserial library
from pwn import *  # Import pwn module from pwntools library
import argparse  # Import argparse module to parse command-line arguments
from sys import platform  # Import platform module from sys library to determine the OS

if platform == "linux" or platform == "linux2":
    # If the OS is Linux
    parser = argparse.ArgumentParser(
        description='NYCU OSDI kernel sender')  # Create an argument parser
    parser.add_argument('--filename', metavar='PATH', default='kernel8.img', type=str,
                        help='path to kernel8.img')  # Add a filename argument with default value 'kernel8.img'
    parser.add_argument('--device', metavar='TTY', default='/dev/ttyUSB0', type=str,
                        help='path to UART device')  # Add a device argument with default value '/dev/ttyUSB0'
    # Add a baud argument with default value 115200
    parser.add_argument('--baud', metavar='Hz',
                        default=115200, type=int,  help='baud rate')
    args = parser.parse_args()  # Parse the arguments

    # Open the file specified by the filename argument in binary read mode
    with open(args.filename, 'rb') as fd:
        # Open a serial connection with the device and baud rate specified by the arguments
        with Serial(args.device, args.baud) as ser:

            kernel_raw = fd.read()  # Read the kernel image into memory
            length = len(kernel_raw)  # Get the length of the kernel image

            # Print the size of the kernel image in hexadecimal
            print("Kernel image size : ", hex(length))
            for i in range(8):
                # Write the length of the kernel image to the serial port, 1 byte at a time
                ser.write(p64(length)[i:i+1])
                ser.flush()  # Flush the output buffer

            print("Start sending kernel image by uart1...")
            for i in range(length):
                # Write the kernel image to the serial port, 1 byte at a time
                # Use kernel_raw[i: i+1] is byte type. Instead of using kernel_raw[i] it will retrieve int type then cause error
                ser.write(kernel_raw[i: i+1])
                ser.flush()  # Flush the output buffer
                if i % 100 == 0:
                    # Print the progress every 100 bytes
                    print("{:>6}/{:>6} bytes".format(i, length))
            # Print the final progress
            print("{:>6}/{:>6} bytes".format(length, length))
            # Print a message indicating that the transfer is complete
            print("Transfer finished!")

else:
    # If the OS is not Linux (assumed to be Windows)
    parser = argparse.ArgumentParser(
        description='NYCU OSDI kernel sender')  # Create an argument parser
    parser.add_argument('--filename', metavar='PATH', default='kernel8.img', type=str,
                        help='path to kernel8.img')  # Add a filename argument with default value 'kernel8.img'
    parser.add_argument('--device', metavar='COM', default='COM3', type=str,
                        help='COM# to UART device')  # Add a device argument with default value 'COM3'
    # Add a baud argument with default value 115200
    parser.add_argument('--baud', metavar='Hz',
                        default=115200, type=int,  help='baud rate')
    args = parser.parse_args()  # Parse the arguments

    with open(args.filename, 'rb') as fd:
        with Serial(args.device, args.baud) as ser:

            kernel_raw = fd.read()
            length = len(kernel_raw)

            print("Kernel image size : ", hex(length))
            for i in range(8):
                ser.write(p64(length)[i:i+1])
                ser.flush()

            print("Start sending kernel image by uart1...")
            for i in range(length):
                # Use kernel_raw[i: i+1] is byte type. Instead of using kernel_raw[i] it will retrieve int type then cause error
                ser.write(kernel_raw[i: i+1])
                ser.flush()
                if i % 100 == 0:
                    print("{:>6}/{:>6} bytes".format(i, length))
            print("{:>6}/{:>6} bytes".format(length, length))
            print("Transfer finished!")
