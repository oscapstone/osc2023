ARMGNU ?= aarch64-linux-gnu

COPS = -Wall -nostdlib -nostartfiles -ffreestanding -I include -mgeneral-regs-only -g
ASMOPS = -Iinclude 

BUILD_DIR = build
KERNEL_DIR = kernel
BOOTLOADER_DIR = bootloader
SRC_DIR = src
HOME_DIR = .
INITRAMFS_CPIO := initramfs.cpio

all : kernel8.img bootloader.img $(INITRAMFS_CPIO)

clean :
	rm -rf $(BUILD_DIR) *.img $(INITRAMFS_CPIO)
	
$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

$(BUILD_DIR)/$(KERNEL_DIR)/%_c.o: $(SRC_DIR)/$(KERNEL_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/$(KERNEL_DIR)/%_s.o: $(SRC_DIR)/$(KERNEL_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_c.o: $(SRC_DIR)/$(BOOTLOADER_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_s.o: $(SRC_DIR)/$(BOOTLOADER_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

$(INITRAMFS_CPIO): $(wildcard rootfs/*)
	cd rootfs; find . | cpio -o -H newc > ../$(INITRAMFS_CPIO)

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

KERNEL_C_FILES = $(wildcard $(SRC_DIR)/$(KERNEL_DIR)/*.c)
KERNEL_ASM_FILES = $(wildcard $(SRC_DIR)/$(KERNEL_DIR)/*.S)

BOOTLOADER_C_FILES = $(wildcard $(SRC_DIR)/$(BOOTLOADER_DIR)/*.c)
BOOTLOADER_ASM_FILES = $(wildcard $(SRC_DIR)/$(BOOTLOADER_DIR)/*.S)

KERNEL_OBJ_FILES = $(KERNEL_C_FILES:$(SRC_DIR)/$(KERNEL_DIR)/%.c=$(BUILD_DIR)/$(KERNEL_DIR)/%_c.o)
KERNEL_OBJ_FILES += $(KERNEL_ASM_FILES:$(SRC_DIR)/$(KERNEL_DIR)/%.S=$(BUILD_DIR)/$(KERNEL_DIR)/%_s.o)

BOOTLOADER_OBJ_FILES = $(BOOTLOADER_C_FILES:$(SRC_DIR)/$(BOOTLOADER_DIR)/%.c=$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_c.o)
BOOTLOADER_OBJ_FILES += $(BOOTLOADER_ASM_FILES:$(SRC_DIR)/$(BOOTLOADER_DIR)/%.S=$(BUILD_DIR)/$(BOOTLOADER_DIR)/%_s.o)

DEP_FILES = $(BOOTLOADER_OBJ_FILES:%.o=%.d) + $(KERNEL_OBJ_FILES:%.o=%.d) + $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: $(SRC_DIR)/$(KERNEL_DIR)/linker.ld $(KERNEL_OBJ_FILES) $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/$(KERNEL_DIR)/linker.ld -o $(BUILD_DIR)/$(KERNEL_DIR)/kernel8.elf  $(KERNEL_OBJ_FILES) $(OBJ_FILES) -g
	$(ARMGNU)-objcopy $(BUILD_DIR)/$(KERNEL_DIR)/kernel8.elf -O binary kernel8.img

bootloader.img: $(SRC_DIR)/$(BOOTLOADER_DIR)/linker.ld $(BOOTLOADER_OBJ_FILES) $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/$(BOOTLOADER_DIR)/linker.ld -o $(BUILD_DIR)/$(BOOTLOADER_DIR)/bootloader.elf  $(BOOTLOADER_OBJ_FILES) $(OBJ_FILES) -g
	$(ARMGNU)-objcopy $(BUILD_DIR)/$(BOOTLOADER_DIR)/bootloader.elf -O binary bootloader.img

debug: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none \
					-initrd initramfs.cpio \
					-dtb bcm2710-rpi-3-b-plus.dtb \
					-serial null -serial stdio \

debuggdb: all
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none -s -S

debugasm: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm

debugb: all
	qemu-system-aarch64 -M raspi3b -kernel bootloader.img -display none \
					-initrd initramfs.cpio \
					-dtb bcm2710-rpi-3-b-plus.dtb \
					-serial null -serial pty