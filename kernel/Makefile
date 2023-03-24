SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

LIBRARY_PATH = ../library
INCLUDES = -I ./include \
	$(LIBRARY_INCLUDE)

LIBRARY_INCLUDE = -I $(LIBRARY_PATH)/include
LIBRARY_SRCS = $(LIBRARY_PATH)/*.c
LIBRARY_OBJS = $(LIBRARY_SRCS:.c=.o)

all: clean kernel8.img

start.o: start.S
	aarch64-linux-gnu-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

kernel8.img: start.o $(OBJS)
	aarch64-linux-gnu-ld start.o $(OBJS) $(LIBRARY_OBJS) -T link.ld -o kernel8.elf
	aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf kernel8.img *.o 2> /dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -display none -initrd initramfs.cpio
