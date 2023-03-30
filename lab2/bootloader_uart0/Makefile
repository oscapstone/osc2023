ARMGNU ?= aarch64-none-elf
CC = $(ARMGNU)-gcc
LD = $(ARMGNU)-ld
OBJCOPY = $(ARMGNU)-objcopy
BUILD_DIR = build
SRC_DIR = src

all : bootloader.img

clean : 
	rm -rf bootloader.img build

COPS = -Wall -nostdlib -nostdinc -nostartfiles -ffreestanding -mgeneral-regs-only

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(COPS) -MMD -g -c $< -o $@

$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.S
	$(CC) $(CFLAGS) -c $(SRC_DIR)/boot.S -o $(BUILD_DIR)/boot.o

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/boot.o)

bootloader.img : $(SRC_DIR)/linker.ld  $(OBJ_FILES)
	echo $(OBJ_FILES)
	$(LD) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/bootloader.elf $(OBJ_FILES)
	$(OBJCOPY) -O binary $(BUILD_DIR)/bootloader.elf bootloader.img

run: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial pty -serial stdio -display none

pty: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial pty -display none

asm: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial pty -serial stdio -d in_asm -display none

# aarch64-linux-gnu-gcc -c a.S
# aarch64-linux-gnu-ld -T linker.ld -o kernel8.elf a.o
# aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.im
# qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm

debug: bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -serial pty -display none -S -s