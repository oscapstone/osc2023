from pwn import *

kernel = open("kernel8.img", "rb").read()
r = serialtube("/dev/pts/4", convert_newlines=False)
#r = serialtube("/dev/ttyS5", convert_newlines=False)
#r = remote('127.0.0.1',10001)
size = (str(len(kernel)) + "\n" ).encode() 
print(size)
r.sendline(size)
print(r.recvuntil(b"): ").decode())
r.send(kernel)
r.interactive()