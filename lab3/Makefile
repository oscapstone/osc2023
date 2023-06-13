AARCH = aarch64-none-elf
SRC_DIR := src
OBJ_DIR := obj


SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CFLAGS = -Wall -O2 -nostdlib
CPPFLAGS := -Iinclude

all: clean kernel8.img

start.o: start.S
	$(AARCH)-gcc $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(AARCH)-gcc $(CPPFLAGS) $(CFLAGS) -c $< -o $@

kernel8.elf: start.o $(OBJS)
	$(AARCH)-ld $^ -T link.ld -o $@

kernel8.img: kernel8.elf
	$(AARCH)-objcopy -O binary $< $@

clean:
	rm kernel8.elf kernel8.img *.o $(OBJ_DIR)/* >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial pty -initrd initramfs.cpio -display none
