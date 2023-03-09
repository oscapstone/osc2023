SRC_DIR :=	src
OBJ_DIR :=	obj

SRCS = 		$(wildcard $(SRC_DIR)/*.c)
OBJS = 		$(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CPPFLAGS := 	-Iinclude -MMD -MP -static
CFLAGS = 	-Wall -O3
LDFLAGS = 	-nostdlib 

CC = /home/elvis/Downloads/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc
LD = /home/elvis/Downloads/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-ld
OBJCOPY = /home/elvis/Downloads/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-elf/bin/aarch64-none-elf-objcopy
QEMU = qemu-system-aarch64


all: clean kernel.img kernel8.img

start.o: start.S 
	$(CC) $(CFLAGS) -c -o $@ $<

boot.o: boot.S | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

kernel8.elf: boot.o $(OBJ_DIR)/uart.o
	$(LD) $(LDFLAGS) $^ -T linker.ld -o $@

kernel.elf:  start.o $(OBJS)
	$(LD) $(LDFLAGS) $^ -T linker.ld -o $@

%.img: %.elf
	$(OBJCOPY) -O binary $< $@ 

initramfs.cpio: write.sh
	ls $^ | cpio -H newc -o > $@

run_asm: kernel8.img kernel.img
	$(QEMU) -M raspi3 -display none -serial none -serial pty -d in_asm -kernel $<

run_dbg: kernel8.img kernel.img
	$(QEMU) -M raspi3 -display none -serial none -serial pty -S -s -kernel $<

run: kernel8.img kernel.img
	$(QEMU) -M raspi3 -display none -serial null -serial pty -kernel $<

run_kernel: kernel.img initramfs.cpio
	$(QEMU) -M raspi3 -display none -serial null -serial stdio -initrd initramfs.cpio -kernel $<

$(OBJ_DIR):
	mkdir -p $@

clean:
	@rm *.img *.o *.elf >/dev/null 2>/dev/null || true
	@rm -r $(OBJ_DIR) >/dev/null 2>/dev/null || true

-include $(OBJS:.o=.d)	
