# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| nigauri1215    | 0816080    | 許舒茵        |

## Lab 2

- [Lab page](https://oscapstone.github.io/)

## How to build
```
make
```
## How to run
```
make run
```

## Bootloader
1. Burn bootloader.img into SD card

2. open rpi3
```
sudo screen /dev/ttyUSB0 115200
```
3. run send_img.py
```
sudo python3 ./script/send_file.py -s kernel8.img
```