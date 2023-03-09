from serial import Serial
from multiprocessing import Process


def transfer_image(serial_port):
    with open('kernel8.img', 'rb') as fd:
        with Serial(serial_port, 115200) as ser:
            raw = fd.read()
            length = len(raw)

            print("Image size: ", length)
            ser.write(str(length).encode() + b'\n')
            ser.flush()

            print("Start to transfer...")
            for i in range(length):
                ser.write(raw[i: i + 1])
                ser.flush()
                if i % 100 == 0:
                    print("{:>6}/{:>6} bytes\n".format(i, length))
            print("{:>6}/{:>6} bytes\n".format(length, length))
            print("Transfer finished!")


if __name__ == '__main__':
    p1 = Process(target=transfer_image, args=('/dev/ttys000',))
    #p2 = Process(target=transfer_image, args=('/dev/tty.usbserial-0001',))
    p1.start()
    #p2.start()
    p1.join()
    #p2.join()
