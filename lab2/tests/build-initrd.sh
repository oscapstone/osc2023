#!/bin/sh -e

out_dir=$(pwd)
cd ../c
find . -mindepth 1 -path ./build -prune -o -printf '%P\n' \
    | cpio -o -H newc > "$out_dir/initramfs.cpio"
