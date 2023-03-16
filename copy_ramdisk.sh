#!/bin/bash

$(cd $1 && find . | cpio -o -H newc > ../initramfs.cpio)
sudo cp initramfs.cpio /mnt/osdi