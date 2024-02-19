#!/bin/sh
cd rootfs
find . | cpio -o -H newc > ../initfs.cp
cd ..