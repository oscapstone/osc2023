# MAC 
ARMPATH  = /Users/stafen/Documents/osc/toolchain/arm-gnu-toolchain-12.2.rel1-darwin-x86_64-aarch64-none-elf/bin
# ARMPATH = /home/hscc/Documents/shun/osc/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf/bin
ARMGCC ?= $(ARMPATH)/aarch64-none-elf-gcc
ARMLD ?= $(ARMPATH)/aarch64-none-elf-ld
ARMOBJCOPY ?= $(ARMPATH)/aarch64-none-elf-objcopy
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -Iinclude
ASMOPS = -Iinclude
BUILD_DIR=out
SRC_DIR=src

all: kernel8.img

clean:
		rm -rf $(BUILD_DIR) *.img

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGCC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
		$(ARMGCC) $(CFLAGS) -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMLD) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMOBJCOPY) $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

run_stdio: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -initrd initramfs.cpio -serial null -serial stdio -dtb bcm2710-rpi-3-b-plus.dtb

run: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null  -initrd initramfs.cpio -serial pty -dtb bcm2710-rpi-3-b-plus.dtb

deploy:
	rm /Volumes/NO\ NAME/kernel8.img
	cp kernel8.img /Volumes/NO\ NAME