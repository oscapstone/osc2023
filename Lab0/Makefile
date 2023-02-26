
all: 
	#From Source Code to Object Files
	clang -mcpu=cortex-a53 --target=aarch64-rpi3-elf -c a.S

	#From Object Files to ELF
	ld.lld -m aarch64elf -T linker.ld -o kernel8.elf a.o

	#From ELF to Kernel Image
	llvm-objcopy --output-target=aarch64-rpi3-elf -O binary kernel8.elf kernel8.img

check:
	#Check on Qemu
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -d in_asm



clean:
	rm kernel8.elf kernel8.img  *.o >/dev/null 2>/dev/null || true

