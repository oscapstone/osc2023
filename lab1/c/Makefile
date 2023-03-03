DEFAULT_PROFILE = DEBUG
SRC_DIR         = src
BUILD_DIR       = build

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

OBJS      = start main delay mailbox mailbox_info reset serial shell libc/string
LD_SCRIPT = $(SRC_DIR)/linker.ld

# ------------------------------------------------------------------------------

PROFILE = $(DEFAULT_PROFILE)
OUT_DIR = $(BUILD_DIR)/$(PROFILE)

CPPFLAGS = $(CPPFLAGS_BASE) $(CPPFLAGS_$(PROFILE))
ASFLAGS  = $(ASFLAGS_BASE) $(ASFLAGS_$(PROFILE))
CFLAGS   = $(CFLAGS_BASE) $(CFLAGS_$(PROFILE))

OBJ_PATHS = $(addprefix $(OUT_DIR)/,$(addsuffix .o,$(OBJS)))

# ------------------------------------------------------------------------------

.PHONY: all qemu qemu-debug gdb clean-profile clean

all: $(OUT_DIR)/kernel8.img

$(OUT_DIR)/kernel8.img: $(OUT_DIR)/kernel8.elf
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $^ $@

$(OUT_DIR)/kernel8.elf: $(OBJ_PATHS) $(LD_SCRIPT)
	@mkdir -p $(@D)
	$(LD) -T $(LD_SCRIPT) $(filter-out $(LD_SCRIPT),$^) -o $@

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
	qemu-system-aarch64 -M raspi3b -kernel $< -display none -serial null \
		-serial stdio

qemu-debug: $(OUT_DIR)/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -display none -serial null \
		-serial stdio -S -s

gdb: $(OUT_DIR)/kernel8.elf
	gdb -s $< -ex 'target remote :1234'

clean-profile:
	$(RM) -r $(OUT_DIR)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(OUT_DIR)/*.d
