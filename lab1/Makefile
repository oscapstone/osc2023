shell = /bin/bash
CROSS_COMPILER := aarch64-linux-gnu-
CC := $(CROSS_COMPILER)gcc
LD := $(CROSS_COMPILER)ld
OC := $(CROSS_COMPILER)objcopy
BUILD_DIR := build
SRC_DIR := src
LIB_DIR := lib
SRCS := $(shell find src lib -name '*.[cs]')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

CFLAGS := -I $(SRC_DIR)/include -I $(LIB_DIR)/include -fno-stack-protector -ffreestanding

.PHONY : all clean

all: kernel8.img

clean:
	rm -rf build kernel8.img kernel8.elf

kernel8.img: $(OBJS)
	echo $(SRCS)
	echo $(OBJS)
	$(LD) -T $(SRC_DIR)/linker.ld -o kernel8.elf $^
	$(OC) -O binary kernel8.elf kernel8.img

$(BUILD_DIR)/src/%.c.o: $(SRC_DIR)/%.c 
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/src/%.s.o: $(SRC_DIR)/%.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.c.o: $(LIB_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%.s.o: $(LIB_DIR)/%.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
