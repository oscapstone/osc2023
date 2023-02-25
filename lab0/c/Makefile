DEFAULT_PROFILE = DEBUG
SRC_DIR         = src
BUILD_DIR       = build

AS      = aarch64-linux-gnu-as
CPP     = aarch64-linux-gnu-cpp
LD      = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

ASFLAGS_DEBUG = -g

OBJS      = a
LD_SCRIPT = $(SRC_DIR)/linker.ld

# ------------------------------------------------------------------------------

PROFILE = $(DEFAULT_PROFILE)
OUT_DIR = $(BUILD_DIR)/$(PROFILE)

ASFLAGS = $(ASFLAGS_BASE) $(ASFLAGS_$(PROFILE))

OBJ_PATHS = $(addprefix $(OUT_DIR)/,$(addsuffix .o,$(OBJS)))

# ------------------------------------------------------------------------------

.PHONY: all qemu clean-profile clean

all: $(OUT_DIR)/kernel8.img

$(OUT_DIR)/kernel8.img: $(OUT_DIR)/kernel8.elf
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $^ $@

$(OUT_DIR)/kernel8.elf: $(OBJ_PATHS)
	@mkdir -p $(@D)
	$(LD) -T $(LD_SCRIPT) $^ -o $@

$(OUT_DIR)/%.o: $(OUT_DIR)/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $^ -o $@

$(OUT_DIR)/%.s: $(SRC_DIR)/%.S
	@mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) $^ -o $@

qemu: $(OUT_DIR)/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -display none -d in_asm

clean-profile:
	$(RM) -r $(OUT_DIR)

clean:
	$(RM) -r $(BUILD_DIR)
