SRC_DIR = lib
OBJ_DIR = build
INCLUDE = include
CFLAGS = -ggdb -Wno-implicit -ffreestanding -nostdlib -nostartfiles -I$(INCLUDE) 
SRC_S:=$(wildcard $(SRC_DIR)/*.S)
SRC_C:=$(wildcard $(SRC_DIR)/*.c)
OBJS:= $(SRC_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) \
	   $(SRC_S:$(SRC_DIR)/%.S=$(OBJ_DIR)/%.o)
MACHINE = raspi3b
CPIO_FILE = ../initramfs.cpio
DTB_FILE = ../bcm2710-rpi-3-b-plus.dtb

IMAGE = kernel

all: $(IMAGE).img 

$(IMAGE).img: $(IMAGE).elf
	aarch64-linux-gnu-objcopy -O binary $(IMAGE).elf $(IMAGE).img

$(IMAGE).elf: $(OBJS) linker.ld
	aarch64-linux-gnu-ld $(OBJS) -T linker.ld -o $(IMAGE).elf

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	@if [ ! -d "$(OBJ_DIR)" ]; then mkdir $(OBJ_DIR); fi
	aarch64-linux-gnu-gcc -o $@ $(CFLAGS) -c $< 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@if [ ! -d "$(OBJ_DIR)" ]; then mkdir $(OBJ_DIR); fi
	aarch64-linux-gnu-gcc -o $@ $(CFLAGS) -c $<

run: $(IMAGE).img 
	qemu-system-aarch64 -machine $(MACHINE) -kernel $(IMAGE).img -initrd $(CPIO_FILE) -dtb $(DTB_FILE) -display none -serial null -serial stdio

dbg: $(IMAGE).img 
	qemu-system-aarch64 -machine $(MACHINE) -kernel $(IMAGE).img -initrd $(CPIO_FILE) -dtb $(DTB_FILE) -display none -serial null -serial stdio -s -S

clean:
	rm -rf $(OBJ_DIR) *.img *.elf */*.elf
