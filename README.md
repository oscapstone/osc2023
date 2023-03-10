# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| RobertttBS     | 311555026  | 謝柏陞         |

## Requirements

* a cross-compiler for aarch64
* (optional) qemu-system-arm

## Build 

```
make
```

## Test With QEMU

```
make run
```

## Send image to Raspi 3

```
make sendimg
```

## Communicate with Raspi 3 after `make sendimg`

```
make commu
```
