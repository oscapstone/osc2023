kernel_std_exe:
	qemu-system-aarch64 -M raspi3b -kernel ../../kernel8.img -serial null -serial stdio -display none -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb
kernel_pty_exe:
	qemu-system-aarch64 -M raspi3b -kernel ../../kernel8.img -serial null -serial pty -display none -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb
bootloader_std_exe:
	qemu-system-aarch64 -M raspi3b -kernel ../../bootloader.img -serial null -serial stdio -display none -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb
bootloader_pty_exe:
	qemu-system-aarch64 -M raspi3b -kernel ../../bootloader.img -serial null -serial pty -display none -initrd ../initramfs.cpio -dtb ../bcm2710-rpi-3-b-plus.dtb