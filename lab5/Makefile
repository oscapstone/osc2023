# Modify from 
# https://github.com/s-matyukevich/raspberry-pi-os/blob/master/src/lesson01/Makefile
ARMGNU ?= aarch64-none-elf

COPS = -Wall -O3 -mgeneral-regs-only -fno-lto -fno-builtin -Iinclude -g #-ggdb #-D__DEBUG
ASMOPS = -Iinclude 

BUILD_DIR = build
SRC_DIR = src

all : kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES) -nostdlib
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

run_normal: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio

debug: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -initrd initramfs.cpio -serial null -serial stdio -display none -S -s 

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio

display: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio

int: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -d int

asm: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -d in_asm