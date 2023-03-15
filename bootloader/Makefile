SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

LIBRARY_PATH = ../library
INCLUDES = -I ./include \
	$(LIBRARY_INCLUDE)

LIBRARY_INCLUDE = -I $(LIBRARY_PATH)/include
LIBRARY_SRCS = $(LIBRARY_PATH)/*.c
LIBRARY_OBJS = $(LIBRARY_SRCS:.c=.o)

all: clean bootloader.img

start.o: start.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

bootloader.img: start.o $(OBJS)
	aarch64-linux-gnu-ld start.o $(OBJS) $(LIBRARY_OBJS) -T link.ld -o bootloader.elf
	aarch64-linux-gnu-objcopy -O binary bootloader.elf bootloader.img

clean:
	rm bootloader.elf bootloader.img *.o 2> /dev/null || true

run:
	# qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial null -serial pty -display none -S -s
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial null -serial pty -display none
