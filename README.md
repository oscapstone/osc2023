# OSC2023

| Github Account | Student ID | Name          |
|----------------|------------|---------------|
| AlaRduTP       | 311552057  | CHEN YING-TA  |

## Requirements

- GNU Make >= 4.4
- LLVM Project >= 15.0.7
    - Clang
    - LLD
    - llvm-objcopy
- (optional) tmux >= 3.3a
- (optional) qemu-system-arm

You can get them on macOS with Homebrew:

```sh
brew install make llvm tmux qemu
```

## Build 

```
make
```

## Test With QEMU

```
make run
```