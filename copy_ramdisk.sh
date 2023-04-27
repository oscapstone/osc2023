#!/bin/bash

cp rootfs/* build/initramfs/
$(cd build/initramfs && find . | cpio -o -H newc > initramfs.cpio)
sudo cp initramfs.cpio /mnt/osdi