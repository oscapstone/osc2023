TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy

CFLAGS = -Wall

.PHONY: all clean asm run

all: kernel8.img clean

a.o: a.S
	$(CC) $(CFLAGS) -c a.S -o a.o

kernel8.img: a.o
	$(LD) -T linker.ld -o kernel8.elf a.o
	$(OBJCPY) -O binary kernel8.elf kernel8.img

clean:
	rm -f *.o 

asm:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -S -s