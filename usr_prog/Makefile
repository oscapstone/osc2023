ARMPATH  = /Users/stafen/Documents/osc/toolchain/arm-gnu-toolchain-12.2.rel1-darwin-x86_64-aarch64-none-elf/bin
ARMGCC ?= $(ARMPATH)/aarch64-none-elf-gcc
ARMLD ?= $(ARMPATH)/aarch64-none-elf-ld
ARMOBJCOPY ?= $(ARMPATH)/aarch64-none-elf-objcopy


all: usr_prog.img

%.o: %.S
	$(ARMGCC) -c $< -ffreestanding

ASM_FILES = $(wildcard *.S)
OBJS := $(ASM_FILES:%.S=%.o)

usr_prog.img: $(OBJS)
	$(ARMLD) -static -o usr_prog.elf $(OBJS)
	$(ARMOBJCOPY) usr_prog.elf -O binary $@

clean:
	rm usr_prog.* *.o