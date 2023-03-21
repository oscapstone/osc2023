SRC_DIR = src
OBJ_DIR = obj

ARMGNU = aarch64-linux-gnu-
CC = $(ARMGNU)gcc
LD = $(ARMGNU)ld
OBJCPY = $(ARMGNU)objcopy

LINKER = $(SRC_DIR)/linker.ld
SRC = $(wildcard $(SRC_DIR)/*.c)
ASM = $(wildcard $(SRC_DIR)/*.s)
OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC)))
OBJ += $(patsubst %.s, $(OBJ_DIR)/%.o, $(notdir $(ASM)))


FLAG = -Wall -nostdlib -I include -c

.PHONY: all

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(FLAG) $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	$(CC) $(FLAG) $< -o $@

all: dir kernel8.img

dir:
	mkdir -p $(OBJ_DIR)

cpio:
	ls program.img | cpio -H newc -o > initramfs.cpio

kernel8.img: $(OBJ)
	$(LD) $(OBJ) -T $(LINKER) -o kernel8.elf
	$(OBJCPY) -O binary kernel8.elf kernel8.img

run: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb 

asm:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm

debug: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -S -s -dtb bcm2710-rpi-3-b-plus.dtb -initrd initramfs.cpio
tty: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial pty

clean:
	rm -rf $(OBJ_DIR) kernel8.*
