# Lab 2

This folder contains code that builds both the bootloader and the kernel.
Both the self-relocating and the non-self-relocating bootloaders may be built.
See the notes below for building/testing/running these binaries.

Two kinds of bootloaders have different loading requirements.
The self-relocating bootloader must be loaded at `0x80000`
and relocates itself to `0x60000` upon loading.
The non-self-relocating bootloader must be loaded at `0x60000`.

## Building

By default, the build commands (for both C and Rust) build
both the bootloader and the kernel.

### Building the C Project

See the [repository-level README](../README.md#building-c-projects)
for more information.

The compiled bootloader image resides in `build/<profile>/bootloader.img`.

By default, the bootloader is self-relocating.
To build the non-self-relocating bootloader,
add `BOOTLOADER_RELOC=` to the end of the build command.
Note that `build/<profile>/bootloader.{elf,img}`
must be removed when switching modes,
or the bootloader may not get rebuilt.

### Building the Rust Project

TBD

## Testing with QEMU

See the [repository-level README](../README.md#test-with-qemu)
for information on testing the kernel.

### Testing the C Project

Note that only the self-relocating bootloader may be tested in this way.
If the files `build/<profile>/bootloader.{elf,img}` exist,
then the following testing commands
may not rebuild the self-relocating bootloader.
Follow the [building instruction](#building-the-c-project)
to rebuild the self-relocating bootloader.

To test the bootloader with QEMU, use the following command.

```sh
make qemu-bootloader PROFILE=<profile>
```

To test the bootloader with QEMU and launch a GDB debug server,
use the following command.

```sh
make qemu-debug-bootloader PROFILE=<profile>
```

### Testing the Rust Project

TBD

## Running on a Raspberry Pi Model 3 B+

When using the self-relocating bootloader,
copy the bootloader image to the boot partition,
and copy [config-bootloader-reloc.txt](config-bootloader-reloc.txt)
to the boot partition and rename it to `config.txt`.

When using the non-self-relocating bootloader,
copy the bootloader image and [config.txt](config.txt) to the boot partition.
