ARMGNU ?= aarch64-linux-gnu
MAKE := make

COPS = -Wall -nostdlib -nostartfiles -ffreestanding -I../include -mgeneral-regs-only -ggdb
ASMOPS = -I ../include -ggdb
INITRAMFS_BUILD_DIR = ../../build/initramfs
KERNEL_BUILD_DIR = ../../build/kernel



all: $(INITRAMFS_BUILD_DIR)/int $(INITRAMFS_BUILD_DIR)/time

$(INITRAMFS_BUILD_DIR)/%_s.o: %.S
	$(ARMGNU)-gcc $(ASMOPS) -c -o $@ $<

# INT_DEP = $(INITRAMFS_BUILD_DIR)/int_s.o
# -include($(INT_DEP))

# $(INITRAMFS_BUILD_DIR)/int: $(INITRAMFS_BUILD_DIR)/int_s.o
# 	$(ARMGNU)-ld -T linker.ld -o $(INITRAMFS_BUILD_DIR)/int.elf $(INITRAMFS_BUILD_DIR)/int_s.o
# 	$(ARMGNU)-objcopy $(INITRAMFS_BUILD_DIR)/int.elf -O binary $(INITRAMFS_BUILD_DIR)/int
# 	rm $(INITRAMFS_BUILD_DIR)/int_s.o
# 	rm $(INITRAMFS_BUILD_DIR)/int.elf

$(INITRAMFS_BUILD_DIR)/int: $(INITRAMFS_BUILD_DIR)/int_s.o
	$(ARMGNU)-ld -T linker.ld -o $@.elf $<
	$(ARMGNU)-objcopy $@.elf -O binary $@
	rm $<
	rm $@.elf
$(INITRAMFS_BUILD_DIR)/time: $(INITRAMFS_BUILD_DIR)/time_s.o
	$(ARMGNU)-ld -T linker.ld -o $@.elf $<
	$(ARMGNU)-objcopy $@.elf -O binary $@
	rm $<
	rm $@.elf