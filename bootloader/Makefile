# MAC
ARMPATH  = /Users/stafen/Documents/osc/toolchain/arm-gnu-toolchain-12.2.rel1-darwin-x86_64-aarch64-none-elf/bin
# ARMPATH = /home/hscc/Documents/shun/osc/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf/bin
ARMGCC ?= $(ARMPATH)/aarch64-none-elf-gcc
ARMLD ?= $(ARMPATH)/aarch64-none-elf-ld
ARMOBJCOPY ?= $(ARMPATH)/aarch64-none-elf-objcopy
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles
# CFLAGS = -g -ffreestanding -nostdinc -nostdlib -nostartfiles
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
all: bootloader.img 

boot.o: boot.S
	$(ARMGCC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(ARMGCC) $(CFLAGS) -c $< -o $@

bootloader.img: boot.o $(OBJS)
	$(ARMLD) -T linker.ld -o bootloader.elf $(OBJS) boot.o
	$(ARMOBJCOPY) bootloader.elf -O binary bootloader.img

clean:
	rm bootloader.* *.o
run_stdio: all
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -serial null -serial stdio

run: all
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -serial null -serial pty

deploy:
	rm /Volumes/NO\ NAME/bootloader.img
	cp bootloader.img /Volumes/NO\ NAME