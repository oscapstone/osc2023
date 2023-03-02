ARMPATH  = /Users/stafen/Documents/osc/toolchain/arm-gnu-toolchain-12.2.rel1-darwin-x86_64-aarch64-none-elf/bin
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

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio

deploy:
	rm /Volumes/NO\ NAME/kernel8.img
	cp kernel8.img /Volumes/NO\ NAME