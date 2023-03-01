export PATH := /usr/local/opt/llvm/bin:$(PATH)

SRC := $(wildcard kernel/*.c)
OBJ := $(SRC:.c=.o) a.o

IMG := kernel8.img
ELF := $(IMG:.img=.elf)

ARC := aarch64-elf
LDF := -m aarch64elf -nostdlib -T linker.ld

CPU := cortex-a53

CFLAGS := --target=$(ARC) -mcpu=$(CPU)

all: clean $(IMG)

$(IMG): $(ELF)
	llvm-objcopy -I $(ARC) -O binary $< $@

$(ELF): $(OBJ)
	ld.lld $(LDF) -o $@ $^

a.o: a.S
	clang $(CFLAGS) -c a.S

%.o: %.c
	clang $(CFLAGS) $(CINCLD) -c $< -o $@

clean:
	rm -rf $(ELF) $(IMG) $(OBJ)

run:
	@qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm