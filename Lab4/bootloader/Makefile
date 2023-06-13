CC 		= clang
LD 		= ld.lld
OBJ_CPY		= llvm-objcopy
LINK_SCRIPT 	= link.ld 
CFLAGS 		= --target=aarch64-elf -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd


SRC_DIR		= src
INCLUDE_DIR = include
OUT_DIR		= build



# cpio archive
CPIO_DIR = rootfs
CPIO_FILES = $(wildcard $(CPIO_DIR)/*)
CPIO = initramfs.cpio
QEMU_CPIO = -initrd $(CPIO)

DTB = bcm2710-rpi-3-b-plus.dtb
QEMU_DTB = -dtb $(DTB)

SRCS 		= $(wildcard $(SRC_DIR)/*.c )
OBJS 		= $(patsubst $(SRC_DIR)/%.c, %.o, $(SRCS))


VPATH = $(SRC_DIR)
vpath %.c %.S $(SRC_DIR) 



boot.img: start.o $(OBJS)
	$(LD) -m aarch64elf -nostdlib $(OUT_DIR)/start.o $(addprefix $(OUT_DIR)/,$(notdir $(OBJS))) -T $(SRC_DIR)/$(LINK_SCRIPT) -o boot.elf
	$(OBJ_CPY) -O binary boot.elf $@

all: boot.img

start.o: start.S
	$(CC) $(CFLAGS) -c $< -o $(OUT_DIR)/start.o

%.o: %.c
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -c $< -o $(OUT_DIR)/$@



# Make Command
run:
	qemu-system-aarch64 -M raspi3b -kernel boot.img -serial null -serial stdio -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb
	
run_pty:
	qemu-system-aarch64 -M raspi3b -kernel boot.img -serial null -serial pty -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb
	

clean:
	rm kernel8.elf boot.img obj/*.o *.o ../*.cpio >/dev/null 2>/dev/null || true

debug:	
	qemu-system-aarch64 -M raspi3b -kernel boot.img -serial null -serial pty -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb -s -S

cpio: 
	cd ../$(CPIO_DIR) ; find . | cpio -o -H newc > ../initramfs.cpio
