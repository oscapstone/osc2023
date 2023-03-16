import sys
import serial
import time
from tqdm import tqdm

def main():
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} <kernel image path> [pts path]")
    use_qe = len(sys.argv) > 2
    with open(sys.argv[1], 'rb') as fin:
        content = fin.read()
    no_hex = hex(len(content))[2:].rjust(16, '0')
    print(f"no_hex: {no_hex}")
    dev = '/dev/ttyUSB0' if not use_qe else sys.argv[2]
    devw = open(dev, "wb", buffering = 0)
    devr = open(dev, "rb", buffering = 0)
    s = b's'
    devw.write(s)
    # while True:
    #     devw.write(s)
    #     time.sleep(0.01)
    #     c = devr.read(1)
    #     if c == s:
    #         break
    print(f"writing length: {no_hex}")
    for b in no_hex.encode():
        time.sleep(0.003)
        devw.write(b.to_bytes(1, 'big'))
        # time.sleep(0.01)
        # recv = devr.read(1)
        # if recv != b'.':
        #     print(f"recv: {recv.decode()} != .")
            
    for i in tqdm(range(len(content))):
        time.sleep(0.003)
        devw.write(content[i].to_bytes(1, 'big'))
        # time.sleep(0.01)
        # recv = devr.read(1)
        # if recv != b'.':
        #     print(f"recv: {recv.decode()} != .")
    devr.close()
    devw.close()
    #with serial.Serial(dev, 115200, timeout=1) as tty:
    input('press any key to end')
    
    
if __name__ == '__main__':
    main()