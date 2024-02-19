# This python script is for macOS
from serial import Serial
import os
import time
import fcntl
import threading

file_size = os.path.getsize('kernel8.img')
print("File Size is :", file_size, "bytes")

with Serial('/dev/tty.usbserial-0001', 115200) as ser:
    sem = threading.Semaphore()
    # set tty to non-blocking mode
    fcntl.fcntl(ser, fcntl.F_SETFL, os.O_NONBLOCK)
  
    input("Press anything to start transmission\n")

    print("Sending Strat...")
    sem.acquire()
    ser.write(b"Start")
    sem.release()
    time.sleep(1)

    sem.acquire()
    ser.write(file_size.to_bytes(4, 'little'))
    sem.release()
    time.sleep(1)

    print("Start sending kernel img by uart...")
    with open('kernel8.img', 'rb') as kernel_file:
        while True:
            data = kernel_file.read()
            if not data:
                break
            sem.acquire()
            ser.write(data)
            sem.release()
            time.sleep(1)

    print("Transfer finished!")
