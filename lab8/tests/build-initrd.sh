#!/bin/sh -e

rm -rf rootfs

mkdir rootfs
wget https://oscapstone.github.io/_downloads/4ee703906d67d0333ef4c215dc060ab3/vfs2.img \
    -O rootfs/vfs2.img

cd rootfs
find . -mindepth 1 | cpio -o -H newc > ../initramfs.cpio

cd ..
rm -r rootfs
