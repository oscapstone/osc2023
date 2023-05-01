BOOTLOADER_IMG = bootloader.img
KERNEL_IMG = kernel8.img
INITRAMFS_CPIO = initramfs.cpio
RPI3_DTB = bcm2710-rpi-3-b-plus.dtb

all: $(KERNEL_IMG)

ARMGNU ?= aarch64-linux-gnu

COPS = -g -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
ASMOPS = -Iinclude

BUILD_DIR = build
SRC_DIR = src
KERNEL_DIR = kernel
BOOTLOADER_DIR = bootloader

all : $(KERNEL_IMG) $(BOOTLOADER_IMG)

.PHONY: clean
clean:
	-rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_c.o: $(KERNEL_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(KERNEL_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_c.o: $(BOOTLOADER_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(BOOTLOADER_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

SRC_C_FILES = $(wildcard $(SRC_DIR)/*.c)
SRC_ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
SRC_OBJ_FILES = $(SRC_C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
SRC_OBJ_FILES += $(SRC_ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

SRC_DEP_FILES = $(SRC_OBJ_FILES:%.o=%.d)
-include $(SRC_DEP_FILES)

KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_ASM_FILES = $(wildcard $(KERNEL_DIR)/*.S)
KERNEL_OBJ_FILES = $(KERNEL_C_FILES:$(KERNEL_DIR)/%.c=$(BUILD_DIR)/%_c.o)
KERNEL_OBJ_FILES += $(KERNEL_ASM_FILES:$(KERNEL_DIR)/%.S=$(BUILD_DIR)/%_s.o)

KERNEL_DEP_FILES = $(KERNEL_OBJ_FILES:%.o=%.d)
-include $(KERNEL_DEP_FILES)

BOOTLOADER_C_FILES = $(wildcard $(BOOTLOADER_DIR)/*.c)
BOOTLOADER_ASM_FILES = $(wildcard $(BOOTLOADER_DIR)/*.S)
BOOTLOADER_OBJ_FILES = $(BOOTLOADER_C_FILES:$(BOOTLOADER_DIR)/%.c=$(BUILD_DIR)/%_c.o)
BOOTLOADER_OBJ_FILES += $(BOOTLOADER_ASM_FILES:$(BOOTLOADER_DIR)/%.S=$(BUILD_DIR)/%_s.o)

BOOTLOADER_DEP_FILES = $(BOOTLOADER_OBJ_FILES:%.o=%.d)
-include $(BOOTLOADER_DEP_FILES)

$(BOOTLOADER_IMG): $(BOOTLOADER_DIR)/linker.ld $(BOOTLOADER_OBJ_FILES) $(SRC_OBJ_FILES)
	$(ARMGNU)-ld -T $(BOOTLOADER_DIR)/linker.ld -o $(BUILD_DIR)/bootloader.elf  $(SRC_OBJ_FILES) $(BOOTLOADER_OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/bootloader.elf -O binary $(BOOTLOADER_IMG)

$(KERNEL_IMG): $(KERNEL_DIR)/linker.ld $(KERNEL_OBJ_FILES) $(SRC_OBJ_FILES)
	$(ARMGNU)-ld -T $(KERNEL_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf  $(SRC_OBJ_FILES) $(KERNEL_OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(KERNEL_IMG)

test_kernel:
	qemu-system-aarch64 -M raspi3b -kernel $(KERNEL_IMG) -serial null -serial stdio -initrd $(INITRAMFS_CPIO) -dtb $(RPI3_DTB) -display none -S -s

test_video:
	qemu-system-aarch64 -M raspi3b -kernel $(KERNEL_IMG) -serial null -serial stdio -initrd $(INITRAMFS_CPIO) -dtb $(RPI3_DTB) -S -s

test:
	#qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm
	#qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -S -s -serial null -serial stdio #lab1
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -initrd $(INITRAMFS_CPIO)  -S -s -serial null -serial pty -dtb bcm2710-rpi-3-b-plus.dtb
	#qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -S -s -serial null -serial stdio -dtb bcm2710-rpi-3-b-plus.dtb
	# qemu-system-aarch64 -M raspi3b -kernel $(BOOTLOADER_IMG) -display none \
	# 					-initrd $(INITRAMFS_CPIO) \
	# 					-dtb $(RPI3_DTB) \
	# 					-chardev pty,id=pty0,logfile=pty.log,signal=off \
	# 				    -serial null -serial chardev:pty0 -s -S
	#qemu-system-aarch64 -M raspi3b -kernel $(KERNEL_IMG) -serial null -serial stdio -initrd $(INITRAMFS_CPIO) -dtb $(RPI3_DTB) -display none -S -s