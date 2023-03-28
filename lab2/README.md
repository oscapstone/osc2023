# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| nigauri1215    | 0816080    | 許舒茵        |

## Lab 2

- [Lab page](https://oscapstone.github.io/)

## Archive New ASCII Format Cpio
```
cd rootfs
find . | cpio -o -H newc > ../initramfs.cpio
cd ..
```
- put cpio archive to SD card

## How to build
```
make
```
## How to run
```
make run
```

## Test With QEMU

```
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -S -s -dtb bcm2710-rpi-3-b-plus.dtb
```

## Sending kernel by Bootloader
1. Burn bootloader.img into SD card

2. open rpi3
```
sudo screen /dev/ttyUSB0 115200
```
3. run send_img.py
```
sudo python3 ./kernel/send_img.py
```