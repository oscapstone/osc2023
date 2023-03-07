# debug use CFLAGS='-g' ASMFLAGS='-g'

ARMGNU ?= aarch64-linux-gnu

COPS = $(CFLAGS) -Wall -nostdlib -nostartfiles -ffreestanding -Iinclude #-ggdb  #-D__FS_DEBUG #-D__DEBUG #-D__DEBUG_MM #-D__DEBUG_MM_ALLOC #-D__DEBUG_MM_SCHED
ASMOPS = $(ASMFLAGS) -Iinclude 

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
	$(ARMGNU)-ld -g -T linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy -g $(BUILD_DIR)/kernel8.elf -O binary kernel8.img

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio

run_debug:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -s -S

connect_raspi:
	sudo screen /dev/ttyUSB0 115200

