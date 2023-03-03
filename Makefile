CROSS_COMPILER ?= aarch64-linux-gnu-
CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld

SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
IMG_DIR = img

CFLAGS  = -Wall -nostdlib -nostartfiles -ffreestanding -mgeneral-regs-only \
				-I$(INC_DIR)
LDFLAGS = -I$(INC_DIR)

KERNEL_IMG := $(IMG_DIR)/kernel8.img
KERNEL_ELF := $(OBJ_DIR)/kernel8.elf

C_FILES     = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES   = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILE    = $(ASM_FILES:$(SRC_DIR)/%.S=$(OBJ_DIR)/%_s.o)
OBJ_FILE   += $(C_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)

$(KERNEL_IMG): $(KERNEL_ELF)
		$(CROSS_COMPILER)objcopy -O binary $^ $(KERNEL_IMG)

$(KERNEL_ELF): $(SRC_DIR)/linker.ld $(OBJ_FILE)
		$(LD) $(LDFLAGS) -T $< -o $(KERNEL_ELF) $(OBJ_FILE)

$(OBJ_DIR)/%_s.o: $(SRC_DIR)/%.S
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%_c.o: $(SRC_DIR)/%.c
		@mkdir -p $(@D)
		$(CC) $(CFLAGS) -c $< -o $@

qemu: $(KERNEL_IMG)
		qemu-system-aarch64 -M raspi3 -kernel $(KERNEL_IMG) -display none\
											-serial null -serial stdio

.PHONY: clean
clean:
		rm -f $(OBJ_FILE) $(KERNEL_ELF)

.PHONY: clean-all
clean-all: clean
		rm -f $(KERNEL_IMG) $(IMG_NAME)