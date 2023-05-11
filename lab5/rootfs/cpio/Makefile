all: 
	find argv_test.img fork_test.img | cpio -o -H newc > ../../initramfs.cpio
clean: ../../initramfs.cpio
	rm ../../initramfs.cpio