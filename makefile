all:
	cd kernel && make
	cd bootloader && make

.PHONY: clean
clean:
	cd kernel && make clean
	cd bootloader && make clean

.PHONY: run
run:
	cd kernel && make run

.PHONY: bl
bl:
	cd bootloader && make

.PHONY: rootfs
rootfs:
	cd rootfs && (find . | cpio -o -H newc > ../initramfs.cpio)
