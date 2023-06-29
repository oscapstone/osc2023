#!/bin/sh -e

rm -rf rootfs

mkdir rootfs
wget https://oscapstone.github.io/_downloads/4a3ff2431ab7fa74536c184270dbe5c0/vm.img \
    -O rootfs/vm.img

cd rootfs
find . -mindepth 1 | cpio -o -H newc > ../initramfs.cpio

cd ..
rm -r rootfs
