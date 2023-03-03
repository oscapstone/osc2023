CC = clang
CFLAGS = -Wall -nostdlib -ffreestanding -mcpu=cortex-a53+nosimd --target=aarch64-rpi3-elf -mgeneral-regs-only -Iinclude -MMD
QEMU = qemu-system-aarch64
QEMU_FLAGS = -dtb bcm2710-rpi-3-b-plus.dtb -m 1G -cpu cortex-a72 -smp 4 -serial null -serial stdio

SRC_DIR = src
BUILD_DIR = build

SRCS_C = $(wildcard $(SRC_DIR)/*.c)
SRCS_ASM = $(wildcard $(SRC_DIR)/*.S)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS_C))
OBJS += $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.o,$(SRCS_ASM))

.PHONY: all, clean, dump, dump-all, run

# switch Raspberry Pi version
# TODO: qemu still not yet supported RPI4
ifdef RPI4
CFLAGS += -DRPI4
else
CFLAGS += -DRPI3
endif

ifdef DBG
CFLAGS += -g
QEMU_FLAGS += -s -S
endif

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

dump: $(BUILD_DIR)/kernel8.elf
	llvm-objdump -d $(BUILD_DIR)/kernel8.elf

dump-all: $(BUILD_DIR)/kernel8.elf
	llvm-objdump -D $(BUILD_DIR)/kernel8.elf

run:
	$(QEMU) -M raspi3b $(QEMU_FLAGS) -kernel kernel8.img
