import serial
import argparse
import time

class serial_connect:
    def __init__(self, port, baud_rate):
        self.port = port
        self.baud_rate = baud_rate
        print("PORT: {}".format(port))
        print("BAUD_RATE: {}".format(baud_rate))
        self.ser = serial.Serial(self.port, self.baud_rate)
    def write(self, bytes):
        self.ser.write(bytes)
    
    def write_int(self, number):
        if number > 2 ** 32 -1 :
            raise "Number can't not larger then 4 bytes!"
        content = number.to_bytes(4, byteorder='big')
        self.ser.write(content)

    def write_bytes(self, content):
        LINE_UP = '\033[1A'
        LINE_CLEAR = '\x1b[2K'  
        total = len(content)
        cur = 1
        for i in content:
            content = i.to_bytes(1,byteorder='big')
            self.ser.write(content)
            # print(i, " ",self.read_int())
            time.sleep(0.001)
            print("PROGRESS: {}/{}".format(cur,total))
            if cur != total:
                print(LINE_UP, end=LINE_CLEAR)
            cur += 1
    def read(self, max_len):
        return self.ser.read(max_len)

    def read_buffer(self):
        return self.read(self.ser.in_waiting)

    def read_buffer_string(self):
        return self._decode_bytes(self.read_buffer())

    def read_line(self):
        return self._decode_bytes(self.ser.readline())

    def read_int(self):
        bytes_to_read = 4
        number_bytes = self.read(bytes_to_read)
        return int.from_bytes(number_bytes, byteorder='big')
    def _decode_bytes(self, bytes_to_decode):
        return bytes_to_decode.decode("ascii")

    def send_kernel(self, file_path):
        with open(file_path, mode='rb') as file:
            kernel = file.read()
            size = len(kernel)
            print("Start sending kernel, SIZE:{}".format(size))
            self.write(b'c')
            
            self.write_int(size)
            time.sleep(1)
            print(self.read_int())
            time.sleep(1)
            self.write_bytes(kernel)
            time.sleep(1)
            self.write(b'c')
            line = self.read_line()
            if not line.startswith("UBOOT"):
                raise "Uncomplete sending the kernel!"
            print("Successfully!")
            

        

def main():
    parser = argparse.ArgumentParser(description='Send message over uart.')
    parser.add_argument('port', type=str, help='Provide the port of uart')
    parser.add_argument('baud_rate', type=int, help='Baud rate of uart')
    parser.add_argument('file_path', type=str, help='Path of the binary')
    args = parser.parse_args()
    PORT = args.port
    BAUD_RATE = args.baud_rate
    FILE_PATH = args.file_path
    ser = serial_connect(PORT, BAUD_RATE)
    ser.send_kernel(FILE_PATH)

    




if __name__ == "__main__":
    main()