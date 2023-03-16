TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC 	= $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy
QEMU = qemu-system-aarch64

CFLAGS = -Wall -O3 -ffreestanding -nostdinc -nostdlib -nostartfiles -I include 
ASMFLAGS = -I include


SRC_DIR = src
BUILD_DIR = build

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.s)
OBJ_FILES = $(ASM_FILES:$(SRC_DIR)/%.s=$(BUILD_DIR)/%.o)
OBJ_FILES += $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

ELF = kernel8.elf
PROGS = kernel8.img

all: $(PROGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	mkdir -p $(@D)
	$(CC) $(ASMFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(ELF): $(OBJ_FILES) $(SRC_DIR)/linker.ld
	$(LD) -T $(SRC_DIR)/linker.ld -o $@ $(OBJ_FILES)

$(PROGS): $(BUILD_DIR)/$(ELF)
	$(OBJCOPY) -O binary $< $@

run: $(PROGS)
	$(QEMU) -M raspi3 -kernel $< -display none -serial null -serial stdio

debug: $(PROGS)
	$(QEMU) -M raspi3 -kernel $< -display none -S -s -serial null -serial stdio

clean:
	rm -rf $(BUILD_DIR) $(PROGS)