AARCH64_PREFIX = aarch64-linux-gnu-
CC = $(AARCH64_PREFIX)gcc
LD = $(AARCH64_PREFIX)ld
OBJCPY = $(AARCH64_PREFIX)objcopy

CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -I../include -mgeneral-regs-only
ASMFLAGS = -I../include

BUILD_DIR = ../build
IMG_DIR = ../img
SRC_DIR = ../src

.PHONY: all clean

all: $(IMG_DIR)/kernel8.img

clean:
	rm -rf $(OBJ_FILES) $(DEP_FILES)
	rm -rf $(IMG_DIR)/kernel8.img $(BUILD_DIR)/kernel8.elf

$(BUILD_DIR)/%_c.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: %.S
	$(CC) $(ASMFLAGS) -MMD -c $< -o $@

C_FILES = $(wildcard *.c)
ASM_FILES = $(wildcard *.S)
OBJ_FILES = $(C_FILES:%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:%.S=$(BUILD_DIR)/%_s.o)

SRC_C_FILES = mini_uart.c string_utils.c
SRC_ASM_FILES = utils.S mm.S
SRC_OBJ_FILES = $(SRC_C_FILES:%.c=$(BUILD_DIR)/%_c.o)
SRC_OBJ_FILES += $(SRC_ASM_FILES:%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

$(IMG_DIR)/kernel8.img: linker.ld $(OBJ_FILES) $(SRC_OBJ_FILES)
	mkdir -p $(@D)
	$(LD) -T linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES) $(SRC_OBJ_FILES)
	$(OBJCPY) $(BUILD_DIR)/kernel8.elf -O binary $@