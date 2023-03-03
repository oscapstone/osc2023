# OSC2023

| Github Account | Student ID | Name       |
|----------------|------------|------------|
| abt8601        | 310551038  | Po-Yi Tsai |

Toy OS for the [Raspberry Pi Model 3 B+][raspi3bp].
Projects for the course [Operating System Capstone][osc2023].

Each lab will be done (hopefully) in both C and Rust.

[raspi3bp]: https://www.raspberrypi.org/products/raspberry-pi-3-model-b-plus/
[osc2023]: https://oscapstone.github.io/

## Requirements

Building C projects requires:

- GCC and binutils for `aarch64-linux-gnu`.

Building Rust projects requires: TBD

Testing the compiled kernels on QEMU requires:

- `qemu-system-aarch64`

## Build

### Building C Projects

```sh
make PROFILE=<profile>
```

`<profile>` can be `DEBUG` or `RELEASE`.
The `DEBUG` profile disables optimizations and enables debug symbols,
while the `RELEASE` profile enables optimizations and disables debug symbols.
The entire `PROFILE=<profile>` part can be omitted, and if so,
the `DEBUG` profile is used.

The compiled kernel image resides in `build/<profile>/kernel8.img`.

### Building Rust Projects

TBD

## Test With QEMU

### Testing C Projects

```sh
make qemu PROFILE=<profile>
```

Again, the entire `PROFILE=<profile>` part can be omitted, and if so,
the `DEBUG` profile is used.
Refer to the section on [building C projects](#building-c-projects)
for the description of the build profiles.

### Testing Rust Projects

TBD
