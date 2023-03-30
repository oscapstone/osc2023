CC=clang
CFLAGS=-mcpu=cortex-a53+nosimd --target=aarch64-rpi3-elf
CFLAGS+=-Wall -O2 -ffreestanding -nostdinc -nostdlib -I../include
ASMFLAGS=-I../include -I.

LIB_DIR=../lib
BUILD_DIR=../build

.PHONY: all clean run run-dbg dump-asm

all: kernel8.img

$(BUILD_DIR)/%_kernel.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%_kernel.o: %.S
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@ -fPIC

SRC=$(wildcard *.c)
ASM=$(wildcard *.S)
LIB:=$(wildcard $(LIB_DIR)/*.c)
LIB:=$(LIB:$(LIB_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS=$(SRC:%.c=$(BUILD_DIR)/%_kernel.o)
OBJS+=$(ASM:%.S=$(BUILD_DIR)/%_kernel.o)

DEPS=$(OBJS:%.o=%.d)
-include $(DEPS)

$(BUILD_DIR)/libmine.a: $(LIB)
	llvm-ar r $@ $^

kernel8.img: linker.ld $(OBJS) $(BUILD_DIR)/libmine.a
	ld.lld -m aarch64elf -nostdlib $(OBJS) -L$(BUILD_DIR) -lmine -T linker.ld -o $(BUILD_DIR)/kernel8.elf
	llvm-objcopy --output-target=aarch64-rpi3-elf $(BUILD_DIR)/kernel8.elf -O binary $@

clean:
	$(RM) -r $(BUILD_DIR) kernel8.elf kernel8.img

dump-asm: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm

run-dbg: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -S -s -serial null -serial stdio
run-tty: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial pty
run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb
