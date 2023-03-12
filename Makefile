CROSS_COMPILER ?= aarch64-linux-gnu-
CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld

SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
IMG_DIR = img
BOOT_DIR = bootloader
KERNEL_DIR = kernel
LIB_DIR = lib
ECHO = echo

CFLAGS  = -Wall -nostdlib -nostartfiles -ffreestanding -mgeneral-regs-only \
				-I$(INC_DIR)
LDFLAGS = -I$(INC_DIR)

KERNEL_IMG := $(IMG_DIR)/kernel8.img
KERNEL_ELF := $(OBJ_DIR)/$(KERNEL_DIR)/kernel8.elf
BOOTLOADER_IMG := $(IMG_DIR)/bootloader.img
BOOTLOADER_ELF := $(OBJ_DIR)/$(BOOT_DIR)/bootloader.elf

RPI3_DTB  := $(IMG_DIR)/bcm2710-rpi-3-b-plus.dtb
INITRAMFS_CPIO := $(IMG_DIR)/initramfs.cpio

KERNEL_C_FILE     = $(wildcard $(SRC_DIR)/$(KERNEL_DIR)/*.c)
KERNEL_ASM_FILE   = $(wildcard $(SRC_DIR)/$(KERNEL_DIR)/*.S)
KERNEL_OBJ_FILE    = $(KERNEL_ASM_FILE:$(SRC_DIR)/$(KERNEL_DIR)/%.S=$(OBJ_DIR)/$(KERNEL_DIR)/%_s.o)
KERNEL_OBJ_FILE   += $(KERNEL_C_FILE:$(SRC_DIR)/$(KERNEL_DIR)/%.c=$(OBJ_DIR)/$(KERNEL_DIR)/%_c.o)
BOOTLOADER_C_FILE     = $(wildcard $(SRC_DIR)/$(BOOT_DIR)/*.c)
BOOTLOADER_ASM_FILE   = $(wildcard $(SRC_DIR)/$(BOOT_DIR)/*.S)
BOOTLOADER_OBJ_FILE  = $(BOOTLOADER_ASM_FILE:$(SRC_DIR)/$(BOOT_DIR)/%.S=$(OBJ_DIR)/$(BOOT_DIR)/%_s.o)
BOOTLOADER_OBJ_FILE += $(BOOTLOADER_C_FILE:$(SRC_DIR)/$(BOOT_DIR)/%.c=$(OBJ_DIR)/$(BOOT_DIR)/%_c.o)
LIB_C_FILE    = $(wildcard $(SRC_DIR)/$(LIB_DIR)/*.c)
LIB_ASM_FILE  = $(wildcard $(SRC_DIR)/$(LIB_DIR)/*.S)
LIB_OBJ_FILE  = $(LIB_ASM_FILE:$(SRC_DIR)/$(LIB_DIR)/%.S=$(OBJ_DIR)/$(LIB_DIR)/%_s.o)
LIB_OBJ_FILE += $(LIB_C_FILE:$(SRC_DIR)/$(LIB_DIR)/%.c=$(OBJ_DIR)/$(LIB_DIR)/%_c.o)
INITRAMFS_FILE = $(wildcard rootfs/*)

# echo:
# 	@$(ECHO) "kernel object file is $(KERNEL_OBJ_FILE)"
#   @$(ECHO) "kernel c file is $(KERNEL_C_FILE)"
#   @$(ECHO) "lib object file is $(LIB_OBJ_FILE)"

all:$(KERNEL_IMG) $(BOOTLOADER_IMG)

$(KERNEL_IMG): $(KERNEL_ELF)
		$(CROSS_COMPILER)objcopy -O binary $^ $(KERNEL_IMG)

$(KERNEL_ELF): $(SRC_DIR)/$(KERNEL_DIR)/linker.ld $(KERNEL_OBJ_FILE) $(LIB_OBJ_FILE)
		$(LD) $(LDFLAGS) -T $< -o $(KERNEL_ELF) $(KERNEL_OBJ_FILE) $(LIB_OBJ_FILE)

$(BOOTLOADER_IMG): $(BOOTLOADER_ELF)
		$(CROSS_COMPILER)objcopy -O binary $^ $(BOOTLOADER_IMG)

$(BOOTLOADER_ELF): $(SRC_DIR)/$(BOOT_DIR)/linker.ld $(BOOTLOADER_OBJ_FILE) $(LIB_OBJ_FILE)
		$(LD) $(LDFLAGS) -T $< -o $(BOOTLOADER_ELF) $(BOOTLOADER_OBJ_FILE) $(LIB_OBJ_FILE)

$(OBJ_DIR)/$(KERNEL_DIR)/%_s.o: $(SRC_DIR)/$(KERNEL_DIR)/%.S
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(KERNEL_DIR)/%_c.o: $(SRC_DIR)/$(KERNEL_DIR)/%.c
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(BOOT_DIR)/%_s.o: $(SRC_DIR)/$(BOOT_DIR)/%.S
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(BOOT_DIR)/%_c.o: $(SRC_DIR)/$(BOOT_DIR)/%.c
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(LIB_DIR)/%_s.o: $(SRC_DIR)/$(LIB_DIR)/%.S
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/$(LIB_DIR)/%_c.o: $(SRC_DIR)/$(LIB_DIR)/%.c
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

$(INITRAMFS_CPIO): $(INITRAMFS_FILE)
		cd rootfs; find . | cpio -o -H newc > ../$(INITRAMFS_CPIO)

qemub: all 
		qemu-system-aarch64 -M raspi3 -kernel $(BOOTLOADER_IMG) -display none \
											-serial null -serial pty
qemuk: all $(INITRAMFS_CPIO) $(RPI3_DTB)
		qemu-system-aarch64 -M raspi3 -kernel $(KERNEL_IMG) -display none \
											-dtb $(RPI3_DTB) \
											-initrd $(INITRAMFS_CPIO) \
											-serial null -serial stdio

qemutty: $(KERNEL_IMG)
		qemu-system-aarch64 -M raspi3 -kernel $(KERNEL_IMG) -display none \
											-serial null -serial pty

.PHONY: clean
clean:
		rm -f $(KERNEL_OBJ_FILE) $(KERNEL_ELF) $(BOOTLOADER_OBJ_FILE) $(BOOTLOADER_ELF) $(LIB_OBJ_FILE) $(INITRAMFS_CPIO)

.PHONY: clean-all
clean-all: clean
		rm -f $(KERNEL_IMG) $(BOOTLOADER_IMG)