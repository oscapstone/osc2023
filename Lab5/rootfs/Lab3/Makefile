cpio: 
	find . | cpio -o -H newc > ../initramfs.cpio

program:
	clang -mcpu=cortex-a53 --target=aarch64-rpi3-elf -c user_program.S
	ld.lld -m aarch64elf -T linker.ld -o user_program.elf user_program.o	
	llvm-objcopy --output-target=aarch64-rpi3-elf -O binary user_program.elf user_program.img