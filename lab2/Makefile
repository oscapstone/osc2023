CC=clang
CFLAGS=-mcpu=cortex-a53+nosimd --target=aarch64-rpi3-elf
CFLAGS+=-Wall -ffreestanding -nostdinc -nostdlib -Iinclude -Ibootloader
ASMFLAGS=-Iinclude

SRC_DIR=bootloader
LIB_DIR=lib
BUILD_DIR=build

.PHONY: all clean run run-dbg dump-asm

all: bootloader.img

$(BUILD_DIR)/%_bootloader.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%_bootloader.o: $(SRC_DIR)/%.S
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@ -fPIC

SRC=$(wildcard $(SRC_DIR)/*.c)
ASM=$(wildcard $(SRC_DIR)/*.S)
LIB:=$(wildcard $(LIB_DIR)/*.c)
LIB:=$(LIB:$(LIB_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS=$(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_bootloader.o)
OBJS+=$(ASM:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_bootloader.o)

DEPS=$(OBJS:%.o=%.d)
-include $(DEPS)

$(BUILD_DIR)/libmine.a: $(LIB)
	llvm-ar r $@ $^

bootloader.img: $(SRC_DIR)/linker.ld $(OBJS) $(BUILD_DIR)/libmine.a
	ld.lld -m aarch64elf -nostdlib $(OBJS) -Lbuild -lmine -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/bootloader.elf
	llvm-objcopy --output-target=aarch64-rpi3-elf $(BUILD_DIR)/bootloader.elf -O binary $@

clean:
	$(RM) -r $(BUILD_DIR) bootloader.elf bootloader.img

dump-asm: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -d in_asm

run-dbg: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -S -s -serial null -serial stdio
run-tty: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -serial null -serial pty -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb
run: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -serial null -serial stdio
run-kernel: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio
