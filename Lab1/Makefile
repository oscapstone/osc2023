
SRC_C:=$(shell find -name '*.c')
OBJS:= $(SRC_C:%.c=%.o) \
		start.o

all: kernel8.img

start.o: start.S
	aarch64-linux-gnu-gcc -c start.S

%.o: %.c
	aarch64-linux-gnu-gcc -fno-stack-protector -o $@ -c $< 

kernel8.img: $(OBJS)
	aarch64-linux-gnu-ld -T linker.ld -o kernel8.elf $(OBJS)
	aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf kernel8.img $(OBJS)
