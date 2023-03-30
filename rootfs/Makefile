INITRAMFS := ../initramfs.cpio

all: clean
	find . | cpio -o -H newc > $(INITRAMFS)

clean:
	rm -f $(INITRAMFS)