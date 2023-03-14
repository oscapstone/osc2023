export PATH := /usr/local/opt/llvm/bin:$(PATH)

BCM := ../src/bcm2835

SRC := $(wildcard *.c) $(BCM)/uart.c
OBJ := $(SRC:.c=.o) head.o

IMG := bootloader.img
ELF := $(IMG:.img=.elf)

ARC := aarch64-elf
LDF := -m aarch64elf -nostdlib -T linker.ld

CPU := cortex-a53

CFLAGS := --target=$(ARC) -mcpu=$(CPU)
CINCLD := -I ../include

all: clean $(IMG)

$(IMG): $(ELF)
	llvm-objcopy -I $(ARC) -O binary $< $@

$(ELF): $(OBJ)
	ld.lld $(LDF) -o $@ $^

head.o: head.S
	clang $(CFLAGS) -c head.S

%.o: %.c
	clang $(CFLAGS) $(CINCLD) -c $< -o $@

clean:
	rm -rf $(ELF) $(IMG) $(OBJ)

run:
	@qemu-system-aarch64 -M raspi3b -kernel $(ELF) -display none -serial null -serial pty