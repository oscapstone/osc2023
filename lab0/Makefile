CC = clang
CFLAGS = -Wall -O2 -nostdlib -ffreestanding -mcpu=cortex-a53 --target=aarch64-rpi3-elf

SRC_DIR = src
BUILD_DIR = build

SRCS_C = $(wildcard $(SRC_DIR)/*.c)
SRCS_ASM = $(wildcard $(SRC_DIR)/*.S)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS_C))
OBJS += $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.o,$(SRCS_ASM))

.PHONY: all, clean, dump, dump-all, run

all: kernel8.img

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

kernel8.img: $(SRC_DIR)/linker.ld $(OBJS)
	ld.lld -m aarch64elf -nostdlib $(OBJS) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf
	llvm-objcopy -O binary $(BUILD_DIR)/kernel8.elf kernel8.img

clean:
	rm -rf $(BUILD_DIR) kernel8.img

dump:
	llvm-objdump -d $(BUILD_DIR)/kernel8.elf

dump-all:
	llvm-objdump -D $(BUILD_DIR)/kernel8.elf

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -d in_asm
