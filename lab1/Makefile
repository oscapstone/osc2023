TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy

SRC_DIR = src
OUT_DIR = out

ENTRY = $(SRC_DIR)/start.S
ENTRY_OBJS = $(OUT_DIR)/start.o
LINKERLD = $(SRC_DIR)/linker.ld

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)
CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles -I include 

all: clean kernel8.img



$(ENTRY_OBJS): $(ENTRY)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel8.img: $(OBJS) $(ENTRY_OBJS)
	$(LD) $(ENTRY_OBJS) $(OBJS) -T $(LINKERLD) -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm -f out/* kernel8.*
run: all
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio
