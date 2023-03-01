# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
|chely1212 | 310831002    | 王謙毓 |

## Requirements

* a cross-compiler for aarch64
```shell=
sudo apt-get install gcc-aarch64-linux-gnu
```

* debug in  qemu-system-aarch64 (optional)
```shell=
wget https://download.qemu.org/qemu-7.2.0.tar.xz
tar xvJf qemu-7.2.0.tar.xz
cd qemu-7.2.0

sudo apt-get install make
sudo apt-get install ninja-build
sudo apt-get install gcc
sudo apt-get install libglib2.0-dev
sudo apt-get install libpixman-1-dev
./configure
make

# add to environment path:
export PATH=/home/chely/qemu-7.2.0/build/:$PATH
```

* debug tool: GDB
```shell=
sudo apt-get install gdb-multiarch
```

## Build
```shell=
make kernel8.img
```

## Test with Qemu
```shell= 
qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio
```

