import os
import time
 
file_size = os.path.getsize('kernel8.img')
print("File Size is :", file_size, "bytes")

# with open('/dev/pts/35', "wb", buffering=0) as tty:
with open('/dev/ttyUSB0', "wb", buffering=0) as tty:
    # send Start
    tty.write(b"Start")
    time.sleep(1)

    # send kernel size 
    tty.write(file_size.to_bytes(4, 'little'))
    time.sleep(1)

    # send kernel
    with open('kernel8.img', 'rb') as kernel_file:
        while True:
            data = kernel_file.read()
            if not data:
                break
            tty.write(data)
            time.sleep(1)

