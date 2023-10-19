DEFAULT_PROFILE = DEBUG
SRC_DIR         = src
BUILD_DIR       = build

AR      = aarch64-linux-gnu-ar -rv
AS      = aarch64-linux-gnu-as
CC      = aarch64-linux-gnu-gcc
CPP     = aarch64-linux-gnu-cpp

CPPFLAGS_BASE = -Iinclude

ASFLAGS_DEBUG = -g

CFLAGS_BASE    = -std=c17 -pedantic-errors -Wall -Wextra -ffreestanding \
                 -mcpu=cortex-a53 -mno-outline-atomics -mstrict-align
CFLAGS_DEBUG   = -g
CFLAGS_RELEASE = -O3 -flto

OBJS      = start ctype errno mbox signal stdio stdlib string sys/mman unistd \
            unistd/syscall __detail/utils/fmt

# ------------------------------------------------------------------------------

PROFILE = $(DEFAULT_PROFILE)
OUT_DIR = $(BUILD_DIR)/$(PROFILE)

CPPFLAGS = $(CPPFLAGS_BASE) $(CPPFLAGS_$(PROFILE))
ASFLAGS  = $(ASFLAGS_BASE) $(ASFLAGS_$(PROFILE))
CFLAGS   = $(CFLAGS_BASE) $(CFLAGS_$(PROFILE))

OBJ_PATHS = $(addprefix $(OUT_DIR)/,$(addsuffix .o,$(OBJS)))

# ------------------------------------------------------------------------------

.PHONY: all clean-profile clean

all: $(OUT_DIR)/libc.a

$(OUT_DIR)/libc.a: $(OBJ_PATHS)
	@mkdir -p $(@D)
	$(AR) $@ $^

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -MMD -MP -MF $(OUT_DIR)/$*.d $< -o $@

$(OUT_DIR)/%.o: $(OUT_DIR)/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $^ -o $@

$(OUT_DIR)/%.s: $(SRC_DIR)/%.S
	@mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) $^ -o $@

clean-profile:
	$(RM) -r $(OUT_DIR)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(OUT_DIR)/*.d
