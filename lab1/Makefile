# Define the compiler and utilities
CC := aarch64-elf-gcc
OBJCOPY := aarch64-elf-objcopy

# Compiler flags
# Assuming your project's structure includes directories like 'include' and 'peripheral' at the root level
CFLAGS := -Wall -Wextra -Wpedantic -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -Iinclude -Iperipheral

# Automatically find all source and assembly files in the current directory and subdirectories
# Note: Update the paths as necessary if your .c and .S files are located in subdirectories
SRCS := $(wildcard *.c) $(wildcard */*.c)
ASMS := $(wildcard *.S) $(wildcard */*.S)
# Convert source filenames to object filenames
OBJS := $(SRCS:.c=.o) $(ASMS:.S=.o)

# Default target: build the kernel image
all: kernel8.img

# From ELF to Kernel Image
# Convert the ELF file to a binary kernel image
kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary $< $@

# From Object Files to ELF
# Link object files into an ELF executable
kernel8.elf: $(OBJS) linker.ld
	$(CC) -T linker.ld -o $@ $(OBJS) $(CFLAGS)

# From Source Code to Object Files
# Compile C source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly source files to object files
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f *.o *.elf *.img
