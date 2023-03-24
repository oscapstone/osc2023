AARCH64_PREFIX = aarch64-linux-gnu-
CC = $(AARCH64_PREFIX)gcc
LD = $(AARCH64_PREFIX)ld
OBJCPY = $(AARCH64_PREFIX)objcopy

.PHONY: all clean

all: usr_prog.bin

%.o: %.S
	$(CC) -c $< -ffreestanding

ASM_FILES = $(wildcard *.S)
OBJS := $(ASM_FILES:%.S=%.o)

usr_prog.bin: $(OBJS)
	$(LD) -T linker.ld -static -o usr_prog.elf $(OBJS)
	$(OBJCPY) usr_prog.elf -O binary $@

clean:
	rm usr_prog.* *.o