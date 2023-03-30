# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| nigauri1215    | 0816080    | 許舒茵        |

## Lab 0: Environment Setup

- [Lab page](https://oscapstone.github.io/labs/lab0.html)


## Install QEMU
- WARNING:The lastest version doesn't support raspi3.
```
sudo apt-get install -y qemu-system-aarch64
```

## Flash Bootable Image to SD Card
```
sudo dd if=nctuos.img of=/dev/sdb
```

## Test With QEMU

```
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -S -s
```

## Interact with Rpi3
```
sudo screen /dev/ttyUSB0 115200
```