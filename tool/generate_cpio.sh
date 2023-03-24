# bash

cd ../kernel/rootfs
find . | cpio -o -H newc > ../initramfs.cpio