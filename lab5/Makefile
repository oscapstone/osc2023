SRCS_ASM 		= $(wildcard ./src/asm/*.S)
SRCS_C 			= $(wildcard ./src/c/*.c)
SRCS_LIB		= $(wildcard ./lib/*.c)
OBJS_ASM 		= $(addprefix ./build/asm/, 	$(notdir $(SRCS_ASM:%.S=%.o)))
OBJS_C 			= $(addprefix ./build/c/, 		$(notdir $(SRCS_C:%.c=%.o)))
OBJS_LIB 		= $(addprefix ./build/lib/, 	$(notdir $(SRCS_LIB:%.c=%.o)))

CFLAGS 			= -Wall -ffreestanding -nostdlib -Iinclude -Ilib
CC 				= aarch64-linux-gnu-gcc
LINKER 			= aarch64-linux-gnu-ld
OBJ_COPY 		= aarch64-linux-gnu-objcopy
EMULATOR		= qemu-system-aarch64

.PHONY: 		clean run deploy

all: 			kernel8.img

build/asm/%.o:	src/asm/%.S
	$(CC) $(CFLAGS) -c $< -o $@

build/c/%.o:	src/c/%.c
	$(CC) $(CFLAGS) -c $< -o $@	

build/lib/%.o: 	lib/%.c
	$(CC) $(CFLAGS) -c $< -o $@	

kernel8.img: 	$(OBJS_ASM) $(OBJS_C) $(OBJS_LIB)
	$(LINKER) -nostdlib $(OBJS_ASM) $(OBJS_C) $(OBJS_LIB) -T link.ld -o kernel8.elf
	$(OBJ_COPY) -O binary kernel8.elf kernel8.img

clean:
	rm  kernel8.elf kernel8.img start.o build/*/*.o >/dev/null 2>/dev/null || true
	

run: 			all
	$(EMULATOR) -M raspi3b -kernel kernel8.img -display none  -serial null -serial stdio -initrd initramfs.cpio

deploy: 		all
	cp ./kernel8.img  /Volumes/NO\ NAME
	cp ./initramfs.cpio  /Volumes/NO\ NAME