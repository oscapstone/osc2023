ARMGNU ?= aarch64-none-elf
CC = $(ARMGNU)-gcc
LD = $(ARMGNU)-ld
OBJCOPY = $(ARMGNU)-objcopy
BUILD_DIR = build
SRC_DIR = src
ENV_HDR = -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

CPPFLAGS = -Iinclude -MMD -MP -static
CFLAGS = -Wall -O3
LDFLAGS = -nostdlib 

all : kernel8.img

clean : 
	rm -rf kernel8.img build

# COPS = -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
# ASMOPS = -Iinclude 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -g -c $< -o $@

$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.S
	$(CC) $(CFLAGS) -c $(SRC_DIR)/boot.S -o $(BUILD_DIR)/boot.o

# $(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
# 	$(CC) $(ASMOPS) -MMD -g -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/boot.o)

kernel8.img : $(SRC_DIR)/linker.ld  $(OBJ_FILES)
	echo $(OBJ_FILES)
	$(LD) $(LDFLAGS) $(OBJ_FILES) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel8.elf kernel8.img

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio ${ENV_HDR}

asm: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -d in_asm ${ENV_HDR}

pty: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial pty ${ENV_HDR}

# aarch64-linux-gnu-gcc -c a.S
# aarch64-linux-gnu-ld -T linker.ld -o kernel8.elf a.o
# aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img
# qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm

debug: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none ${ENV_HDR} -S -s