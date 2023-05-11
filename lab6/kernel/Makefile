AARCH64_PREFIX = aarch64-linux-gnu-
CC = $(AARCH64_PREFIX)gcc
LD = $(AARCH64_PREFIX)ld
OBJCPY = $(AARCH64_PREFIX)objcopy
GDB = $(AARCH64_PREFIX)gdb

CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -I../include -mgeneral-regs-only
ASMFLAGS = -I../include

BUILD_DIR = ../build

.PHONY: all clean qemu gdb_start

all: kernel8.img

clean:
	rm kernel8.img $(OBJ_FILES) $(BUILD_DIR)/$(DEP_FILES) \
	../build/kernel8.elf

$(BUILD_DIR)/%_c.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: %.S
	$(CC) $(ASMFLAGS) -MMD -c $< -o $@

C_FILES = $(wildcard *.c)
ASM_FILES = $(wildcard *.S)
OBJ_FILES = $(C_FILES:%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:%.S=$(BUILD_DIR)/%_s.o)

LIBC_FILES = $(wildcard ../lib/*.c)
LIBASM_FILES = $(wildcard ../lib/*.S)
LIBOBJ_FILES = $(LIBC_FILES:../lib/%.c=$(BUILD_DIR)/%_c.o)
LIBOBJ_FILES += $(LIBASM_FILES:../lib/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: linker.ld $(OBJ_FILES) $(LIBOBJ_FILES)
	$(LD) -T linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES) $(LIBOBJ_FILES) 
	$(OBJCPY) $(BUILD_DIR)/kernel8.elf -O binary $@

qemu: # all in one qemu command
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -S -s -serial null -serial pty \
	-initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb

gdb_start:
	$(GDB) $(BUILD_DIR)/kernel8.elf
