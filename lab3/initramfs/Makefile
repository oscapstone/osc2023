ARMGNU ?= aarch64-none-elf
CC = $(ARMGNU)-gcc
LD = $(ARMGNU)-ld
OBJCOPY = $(ARMGNU)-objcopy

CPPFLAGS = -Iinclude -MMD -MP -static
CFLAGS = -Wall -O3
LDFLAGS = -nostdlib 

all : usr.img

clean : 
	rm -rf usr.img usr.d usr.o usr.elf

pack : usr.img
	rm usr.d usr.o usr.elf
	find . | cpio -o -H newc > ../initramfs.cpio

usr.img : 
	$(CC) $(CPPFLAGS) $(CFLAGS) -c usr.S -o usr.o
	$(LD) ${LDFLAGS} usr.o -T linker.ld -o usr.elf 
	$(OBJCOPY) -O binary usr.elf usr.img