DEFAULT_PROFILE  = DEBUG
BOOTLOADER_RELOC = 1
SRC_DIR          = src
BUILD_DIR        = build

AS      = aarch64-linux-gnu-as
CC      = aarch64-linux-gnu-gcc
CPP     = aarch64-linux-gnu-cpp
LD      = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

CPPFLAGS_BASE = -Iinclude

ASFLAGS_DEBUG = -g

CFLAGS_BASE    = -std=c17 -pedantic-errors -Wall -Wextra -ffreestanding \
                 -mno-outline-atomics -mstrict-align
CFLAGS_DEBUG   = -g
CFLAGS_RELEASE = -O3 -mcpu=cortex-a53

OBJS                 = start main delay devicetree initrd mailbox mailbox_info \
                       reset serial shell simple_malloc libc/string
OBJS_BOOTLOADER      = $(if $(BOOTLOADER_RELOC),bootloader-reloc) start \
                       bootloader-main bootloader_crc32 delay serial
LD_SCRIPT            = $(SRC_DIR)/linker.ld
LD_SCRIPT_BOOTLOADER = $(SRC_DIR)/bootloader$(if $(BOOTLOADER_RELOC),-reloc).ld

# ------------------------------------------------------------------------------

PROFILE = $(DEFAULT_PROFILE)
OUT_DIR = $(BUILD_DIR)/$(PROFILE)

CPPFLAGS = $(CPPFLAGS_BASE) $(CPPFLAGS_$(PROFILE))
ASFLAGS  = $(ASFLAGS_BASE) $(ASFLAGS_$(PROFILE))
CFLAGS   = $(CFLAGS_BASE) $(CFLAGS_$(PROFILE))

OBJ_PATHS            = $(addprefix $(OUT_DIR)/,$(addsuffix .o,$(OBJS)))
OBJ_PATHS_BOOTLOADER = $(addprefix $(OUT_DIR)/,$(addsuffix .o,$(OBJS_BOOTLOADER)))

# ------------------------------------------------------------------------------

.PHONY: all qemu qemu-debug gdb clean-profile clean

all: $(OUT_DIR)/kernel8.img $(OUT_DIR)/bootloader.img

$(OUT_DIR)/%.img: $(OUT_DIR)/%.elf
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $^ $@

$(OUT_DIR)/kernel8.elf: $(OBJ_PATHS) $(LD_SCRIPT)
	@mkdir -p $(@D)
	$(LD) -T $(LD_SCRIPT) $(filter-out $(LD_SCRIPT),$^) -o $@

$(OUT_DIR)/bootloader.elf: $(OBJ_PATHS_BOOTLOADER) $(LD_SCRIPT)
	@mkdir -p $(@D)
	$(LD) -T $(LD_SCRIPT_BOOTLOADER) $(filter-out $(LD_SCRIPT),$^) -o $@

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -MMD -MP -MF $(OUT_DIR)/$*.d $< -o $@

$(OUT_DIR)/%.o: $(OUT_DIR)/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $^ -o $@

$(OUT_DIR)/%.s: $(SRC_DIR)/%.S
	@mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) $^ -o $@

qemu: $(OUT_DIR)/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -initrd ../tests/initramfs.cpio \
		-dtb ../tests/bcm2710-rpi-3-b-plus.dtb -display none -serial null \
		-serial stdio

qemu-debug: $(OUT_DIR)/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -initrd ../tests/initramfs.cpio \
		-dtb ../tests/bcm2710-rpi-3-b-plus.dtb -display none -serial null \
		-serial stdio -S -s

qemu-bootloader: $(OUT_DIR)/bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel $< -initrd ../tests/initramfs.cpio \
		-dtb ../tests/bcm2710-rpi-3-b-plus.dtb -display none -serial null \
		-serial pty

qemu-debug-bootloader: $(OUT_DIR)/bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel $< -initrd ../tests/initramfs.cpio \
		-dtb ../tests/bcm2710-rpi-3-b-plus.dtb -display none -serial null \
		-serial pty -S -s

gdb: $(OUT_DIR)/kernel8.elf
	gdb -s $< -ex 'target remote :1234'

clean-profile:
	$(RM) -r $(OUT_DIR)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(OUT_DIR)/*.d
