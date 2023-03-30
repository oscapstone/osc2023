ARMGNU ?= aarch64-none-elf
BUILD_DIR = build
SRC_DIR = src

all : kernel8.img

clean : 
	rm -rf kernel8.img build

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	mkdir -p $(BUILD_DIR)
	$(ARMGNU)-gcc -g -c $< -o $@

ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)

kernel8.img : $(SRC_DIR)/linker.ld  $(OBJ_FILES)
	echo $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGNU)-objcopy -O binary $(BUILD_DIR)/kernel8.elf kernel8.img

test: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm

# aarch64-linux-gnu-gcc -c a.S
# aarch64-linux-gnu-ld -T linker.ld -o kernel8.elf a.o
# aarch64-linux-gnu-objcopy -O binary kernel8.elf kernel8.img
# qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm
