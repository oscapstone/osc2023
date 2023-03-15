export PATH := /usr/local/opt/llvm/bin:$(PATH)

SRC := $(wildcard src/*/*.c) $(wildcard src/*.c)
OBJ := $(SRC:.c=.o) a.o

IMG := kernel8.img
ELF := $(IMG:.img=.elf)

ARC := aarch64-elf
LDF := -m aarch64elf -nostdlib -T linker.ld

CPU := cortex-a53

CFLAGS := --target=$(ARC) -mcpu=$(CPU) -fno-builtin
CINCLD := -I ./include

.PHONY: clean run bootloader bootloader-run rootfs tools clean-all

all: clean $(IMG)

$(IMG): $(ELF)
	llvm-objcopy -I $(ARC) -O binary $< $@

$(ELF): $(OBJ)
	ld.lld $(LDF) -o $@ $^

a.o: a.S
	clang $(CFLAGS) -c a.S

%.o: %.c
	clang $(CFLAGS) $(CINCLD) -c $< -o $@

clean:
	rm -rf $(ELF) $(IMG) $(OBJ)

run:
	@qemu-system-aarch64 -M raspi3b -kernel $(IMG) -display none -serial null -serial stdio -initrd initramfs.cpio

bootloader:
	$(MAKE) -C bootloader

bootloader-run:
	$(MAKE) -C bootloader run

rootfs:
	$(MAKE) -C rootfs

tools:
	$(MAKE) -C tools

clean-all: clean
	$(MAKE) -C bootloader clean
	$(MAKE) -C rootfs clean
	$(MAKE) -C tools clean
