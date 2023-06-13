SRCS = $(wildcard *.c)
ASMS = $(wildcard *.S)

BUILD_DIR = build
OBJS = $(ASMS:%.S=$(BUILD_DIR)/%.o)
CFLAGS = -Wall -nostdlib -nostartfiles -ffreestanding -g
# -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -g
# Can't use mailbox  version => -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -g

all: program.img  



$(BUILD_DIR)/%.o: %.S
	mkdir -p build
	aarch64-linux-gnu-gcc $(CFLAGS) -c $^ -o $@


program.img: $(OBJS)
	aarch64-linux-gnu-ld -static -o program.elf $(OBJS)
	aarch64-linux-gnu-objcopy program.elf -O binary  $@
	cp ./program.img ../rootfs/

clean:
	rm -rf $(BUILD_DIR) program.elf program.img
	clear
