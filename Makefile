ARMGNU ?= aarch64-linux-gnu

INITRAMFS_BUILD_DIR = $(BUILD_DIR)/initramfs
INITRAMFS_SRC_DIR = $(SRC_DIR)/initramfs

MAKE = make

COPS = -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only -ggdb
ASMOPS = -Iinclude -ggdb -mtune=cortex-a53

SRC_DIR = src
BUILD_DIR = build
KERNEL_BUILD_DIR = $(BUILD_DIR)/kernel
KERNEL_SRC_DIR = $(SRC_DIR)/kernel

BOOTLOADER_BUILD_DIR = $(BUILD_DIR)/bootloader
BOOTLOADER_SRC_DIR = $(SRC_DIR)/bootloader


.PHONY: all clean kernel bootloader debug send $(INITRAMFS_BUILD_DIR)
all : kernel8.img bootloader.img $(INITRAMFS_BUILD_DIR)

kernel: kernel8.img

bootloader: bootloader.img

clean :
	rm -rf $(KERNEL_BUILD_DIR) $(BOOTLOADER_BUILD_DIR) $(BUILD_DIR) $(INITRAMFS_BUILD_DIR) *.img

# run :
# 	make && qemu-system-aarch64 -M raspi3b    -cpu cortex-a72    -dtb bcm2710-rpi-3-b-plus.dtb    -m 1G -smp 4 -serial null -serial stdio -kernel kernel8.img -display none
run: kernel8.img bootloader.img $(INITRAMFS_BUILD_DIR)
	qemu-system-aarch64 -M raspi3b   -serial null -serial stdio -kernel kernel8.img -display none

boot: kernel8.img bootloader.img $(INITRAMFS_BUILD_DIR)
	cd build/initramfs && find . | cpio -o -H newc > ../../initramfs.cpio
	qemu-system-aarch64 -M raspi3b -cpu cortex-a72  -serial null -serial pty -kernel bootloader.img -display none -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -d int

copy:
	make && sudo ./copy_kernel.sh ./bootloader.img && sudo ./copy_ramdisk.sh build/initramfs

send:
	make kernel && sudo python3 send_kernel.py $(SER_DEV)

$(KERNEL_BUILD_DIR)/%_s.o: $(KERNEL_SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c -o $@ $<

$(BOOTLOADER_BUILD_DIR)/%_s.o: $(BOOTLOADER_SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c -o $@ $<

$(KERNEL_BUILD_DIR)/%_c.o: $(KERNEL_SRC_DIR)/%.c
	$(ARMGNU)-gcc $(COPS) -MMD -c -o $@ $<

$(BOOTLOADER_BUILD_DIR)/%_c.o: $(BOOTLOADER_SRC_DIR)/%.c
	$(ARMGNU)-gcc $(COPS) -MMD -c -o $@ $<

KERNEL_C_FILES = $(wildcard $(KERNEL_SRC_DIR)/*.c)
KERNEL_ASM_FILES = $(wildcard $(KERNEL_SRC_DIR)/*.S)
KERNEL_OBJ_FILES = $(KERNEL_C_FILES:$(KERNEL_SRC_DIR)/%.c=$(KERNEL_BUILD_DIR)/%_c.o)
KERNEL_OBJ_FILES += $(KERNEL_ASM_FILES:$(KERNEL_SRC_DIR)/%.S=$(KERNEL_BUILD_DIR)/%_s.o)
BOOTLOADER_C_FILES += $(wildcard $(BOOTLOADER_SRC_DIR)/*.c)
BOOTLOADER_ASM_FILES += $(wildcard $(BOOTLOADER_SRC_DIR)/*.S)
BOOTLOADER_OBJ_FILES += $(BOOTLOADER_C_FILES:$(BOOTLOADER_SRC_DIR)/%.c=$(BOOTLOADER_BUILD_DIR)/%_c.o)
BOOTLOADER_OBJ_FILES += $(BOOTLOADER_ASM_FILES:$(BOOTLOADER_SRC_DIR)/%.S=$(BOOTLOADER_BUILD_DIR)/%_s.o)

DEP_FILES = $(KERNEL_OBJ_FILES:%.o=%.d)
DEP_FILES += $(BOOTLOADER_OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: $(KERNEL_SRC_DIR)/linker.ld $(KERNEL_OBJ_FILES)
	mkdir -p $(KERNEL_BUILD_DIR)
	$(ARMGNU)-ld -T $(KERNEL_SRC_DIR)/linker.ld -o $(KERNEL_BUILD_DIR)/kernel8.elf $(KERNEL_OBJ_FILES)
	$(ARMGNU)-objcopy $(KERNEL_BUILD_DIR)/kernel8.elf -O binary kernel8.img

bootloader.img: $(BOOTLOADER_SRC_DIR)/linker.ld $(BOOTLOADER_OBJ_FILES)
	$(ARMGNU)-ld -T $(BOOTLOADER_SRC_DIR)/linker.ld -o $(BOOTLOADER_BUILD_DIR)/bootloader.elf $(BOOTLOADER_OBJ_FILES)
	$(ARMGNU)-objcopy $(BOOTLOADER_BUILD_DIR)/bootloader.elf -O binary bootloader.img

$(INITRAMFS_BUILD_DIR): 
	$(MAKE) -C $(INITRAMFS_SRC_DIR)

$(shell mkdir -p $(KERNEL_BUILD_DIR))
$(shell mkdir -p $(BOOTLOADER_BUILD_DIR))
$(shell mkdir -p $(INITRAMFS_BUILD_DIR))
