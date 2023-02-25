CC=clang
CFLAGS=-mcpu=cortex-a53+nosimd --target=aarch64-rpi3-elf
CFLAGS+=-Wall -O2 -ffreestanding -nostdinc -nostdlib -Iinclude

SRC_DIR=src
BUILD_DIR=build

.PHONY: all clean run run-dbg dump-asm

all: kernel8.img

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	$(CC) $(CFLAGS) -MMD -c $< -o $@

SRC=$(wildcard $(SRC_DIR)/*.c)
ASM=$(wildcard $(SRC_DIR)/*.S)
OBJS=$(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS+=$(ASM:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)

DEPS=$(OBJS:%.o=%.d)
-include $(DEPS)

kernel8.img: $(SRC_DIR)/linker.ld $(OBJS)
	ld.lld -m aarch64elf -nostdlib $(OBJS) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf
	llvm-objcopy --output-target=aarch64-rpi3-elf $(BUILD_DIR)/kernel8.elf -O binary $@

clean:
	$(RM) -r $(BUILD_DIR) kernel8.elf kernel8.img

dump-asm: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm

run-dbg: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -S -s -serial null -serial stdio
run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio

