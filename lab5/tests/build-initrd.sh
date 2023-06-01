#!/bin/sh -e

if [ -e rootfs ]; then
    rm -r rootfs
fi

progs='xcpt_test'

mkdir rootfs
for prog in $progs; do
    make -C "user-program/$prog" PROFILE=RELEASE
    cp "user-program/$prog/build/RELEASE/$prog.img" "rootfs/$prog"
done

cd rootfs
find . -mindepth 1 | cpio -o -H newc > ../initramfs.cpio

cd ..
rm -r rootfs
