AARCH64_PREFIX = aarch64-linux-gnu-
CC = $(AARCH64_PREFIX)gcc

CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -I../include -mgeneral-regs-only
ASMFLAGS = -I../include

BUILD_DIR = ../build

.PHONY: all clean

$(BUILD_DIR)/%_c.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: %.S
	$(CC) $(ASMFLAGS) -MMD -c $< -o $@

C_FILES = $(wildcard *.c)
ASM_FILES = $(wildcard *.S)
OBJ_FILES = $(C_FILES:%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

all: $(OBJ_FILES)

clean:
	rm  $(OBJ_FILES) $(BUILD_DIR)/$(DEP_FILES)
