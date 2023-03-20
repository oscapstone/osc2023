export PATH := /usr/local/opt/llvm/bin:$(PATH)

SRC := $(wildcard src/*/*.c) $(wildcard src/*.c)
OBJ := $(SRC:.c=.o) head.o

IMG := kernel8.img
ELF := $(IMG:.img=.elf)

ARC := aarch64-elf
LDF := -m aarch64elf -nostdlib -T linker.ld

CPU := cortex-a53

CFLAGS := --target=$(ARC) -mcpu=$(CPU) -fno-builtin
CINCLD := -I ./include

BTLDER := bootloader.img
INITRD := initramfs.cpio
DTB := bcm2710-rpi-3-b-plus.dtb

TOOLS := $(patsubst tools/%.c,tool-%,$(wildcard tools/*.c))
LDER_HELPER := tool-loader

RUN_ESSENTIAL := $(BTLDER) $(INITRD) $(DTB) $(IMG) $(LDER_HELPER)

.PHONY: kernel run-kernel run clean-kernel clean-tools clean

all: clean $(RUN_ESSENTIAL)

kernel: clean-kernel $(IMG)

$(IMG): $(ELF)
	llvm-objcopy -I $(ARC) -O binary $< $@

$(ELF): $(OBJ)
	ld.lld $(LDF) -o $@ $^

head.o: head.S
	clang $(CFLAGS) -c $<

%.o: %.c
	clang $(CFLAGS) $(CINCLD) -c $< -o $@

$(BTLDER):
	@$(MAKE) -C bootloader

$(INITRD):
	@$(MAKE) -C rootfs

$(DTB):
	wget https://github.com/raspberrypi/firmware/raw/master/boot/bcm2710-rpi-3-b-plus.dtb

$(TOOLS): tool-%: tools/%.c
	@$(MAKE) -C tools $@

run-kernel: $(IMG) $(INITRD) $(DTB)
	@qemu-system-aarch64 -M raspi3b -kernel $(IMG) -display none -serial null -serial stdio -initrd $(INITRD) -dtb $(DTB)

run: $(RUN_ESSENTIAL)
	@./run.sh $(BTLDER) $(LDER_HELPER) $(INITRD) $(DTB)

clean-kernel:
	rm -rf $(OBJ) $(ELF) $(IMG)

clean-tools:
	rm -rf $(TOOLS)

clean: clean-kernel clean-tools
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C rootfs clean