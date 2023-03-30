# cross-compiler prefix
ARMGNU ?= aarch64-linux-gnu

COPS = -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only
ASMOPS = -Iinclude
# -Wall show all warnings
# -nostdlib do not use C standard lib
# -nostartfiles do not use standard startup files, do this by ourself
# -ffreestanding let compiler to not assume standard func have usual define
# -Iinclude search for header files in the include folder
# -mgeneral-regs-only use only general-purpose registers without NEON reg
# compiled object files
BUILD_DIR = build
# source code
SRC_DIR = src
# get all .c file under dir
C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
# .c to .o, % used to avoid output same target filename with diff input file
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

# compileing .c to .o # $(@D) means the dir of $@
$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@
# compileing .S to .o
# -MMD instructs the gcc compiler to create a dependency file for each generated object file.
$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@


kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

# default make target
all : clean kernel8.img
# make clean: del all compiled file and .img
clean :
	rm -rf $(BUILD_DIR) *.img
run : all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -initrd ./create_fs/initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb


#---
# warn1: only tab no 4 space
# warn2: each command end no space no comment

