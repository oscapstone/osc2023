#!/bin/sh -e

rm -rf rootfs

mkdir rootfs
wget https://oscapstone.github.io/_downloads/3cb3bdb8f851d1cf29ac6f4f5d585981/vfs1.img \
    -O rootfs/vfs1.img

cd rootfs
find . -mindepth 1 | cpio -o -H newc > ../initramfs.cpio

cd ..
rm -r rootfs
