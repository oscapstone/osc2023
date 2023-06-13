#!bin/bash

aarch64-linux-gnu-gcc -c user_exception_test.S
aarch64-linux-gnu-ld -T linker.ld -o user_exception_test.elf user_exception_test.o
aarch64-linux-gnu-objcopy -O binary user_exception_test.elf user_exception_test.img