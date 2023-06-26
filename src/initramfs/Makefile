ARMGNU ?= aarch64-linux-gnu
MAKE := make

COPS = -Wall -nostdlib -nostartfiles -ffreestanding -I../include -mgeneral-regs-only -ggdb -mcmodel=tiny -std=gnu11
ASMOPS = -I../include -ggdb
INITRAMFS_BUILD_DIR = ../../build/initramfs
KERNEL_BUILD_DIR = ../../build/kernel



all: $(INITRAMFS_BUILD_DIR)/int $(INITRAMFS_BUILD_DIR)/time $(INITRAMFS_BUILD_DIR)/sys_test $(INITRAMFS_BUILD_DIR)/test $(INITRAMFS_BUILD_DIR)/hello

$(INITRAMFS_BUILD_DIR)/%_s.o: %.S
	$(ARMGNU)-gcc $(ASMOPS) -c -o $@ $<

$(INITRAMFS_BUILD_DIR)/%_c.o: %.c
	$(ARMGNU)-gcc $(COPS) -c -o $@ $<
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

$(INITRAMFS_BUILD_DIR)/sys_test: $(INITRAMFS_BUILD_DIR)/sys_test_c.o
	$(ARMGNU)-ld -T linker.ld -o $@.elf $<
	$(ARMGNU)-objcopy $@.elf -O binary $@
	rm $<
	# rm $@.elf

$(INITRAMFS_BUILD_DIR)/test: $(INITRAMFS_BUILD_DIR)/test_s.o
	$(ARMGNU)-ld -T linker.ld -o $@.elf $<
	$(ARMGNU)-objcopy $@.elf -O binary $@
	rm $<
	# rm $@.elf

$(INITRAMFS_BUILD_DIR)/hello: $(INITRAMFS_BUILD_DIR)/hello_s.o
	$(ARMGNU)-ld -T linker.ld -o $@.elf $<
	$(ARMGNU)-objcopy $@.elf -O binary $@
	rm $<
	# rm $@.elf