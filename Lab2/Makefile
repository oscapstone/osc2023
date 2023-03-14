ARMGNU ?= aarch64-linux-gnu

COPS = -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only -g
ASMOPS = -Iinclude 

BUILD_DIR = build
SRC_DIR = src

all : kernel8.img bootloader.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: $(SRC_DIR)/kernel/link.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/kernel/link.ld -o $(BUILD_DIR)/kernel/kernel8.elf   $(filter $(BUILD_DIR)/kernel/%_c.o $(BUILD_DIR)/kernel/%_s.o $(BUILD_DIR)/lib/%_c.o $(BUILD_DIR)/lib/%_s.o, $(OBJ_FILES))
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel/kernel8.elf -O binary kernel8.img

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -display none -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

debug:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -display none -s -S -initrd initramfs.cpio

bootloader.img: $(SRC_DIR)/bootloader/link.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/bootloader/link.ld -o $(BUILD_DIR)/bootloader/bootloader.elf  $(filter $(BUILD_DIR)/bootloader/%_c.o $(BUILD_DIR)/bootloader/%_s.o $(BUILD_DIR)/lib/%_c.o $(BUILD_DIR)/lib/%_s.o, $(OBJ_FILES))
	$(ARMGNU)-objcopy -O binary $(BUILD_DIR)/bootloader/bootloader.elf bootloader.img

test:
	@echo Hello

pseudoTTY:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -serial null -serial pty -display none -initrd initramfs.cpio
