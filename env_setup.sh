#!/bin/bash
# install cross-compiler
# name: aarch64-linux-gnu-gcc
sudo apt install gcc make gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
# install Qemu
sudo apt-get install -y qemu-system-aarch64 gdb-multiarch