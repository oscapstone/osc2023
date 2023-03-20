AARCH = aarch64-none-elf
SRC_DIR := src
OBJ_DIR := obj

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CFLAGS = -Wall -O2 -nostdlib

CPPFLAGS := -Iinclude

all: clean kernel.img

start.o: start.S
	$(AARCH)-gcc $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(AARCH)-gcc $(CPPFLAGS) $(CFLAGS) -c $< -o $@

cpio:
	ls *.md *.c *.h | cpio -H hpodc -o  >ramdisk

rd.o: ramdisk
	$(AARCH)-ld -r -b binary -o $@ $<


kernel.elf: start.o rd.o $(OBJS)
	$(AARCH)-ld $^ -T link.ld -o $@


kernel.img: kernel.elf
	$(AARCH)-objcopy -O binary $< $@

clean:
	rm kernel.elf kernel.img *.o $(OBJ_DIR)/* >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel.img -serial null -serial stdio -display none -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

