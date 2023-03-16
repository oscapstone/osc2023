CC      = aarch64-linux-gnu-gcc
LD      = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

CFLAGS   = -Wall -nostdlib -nostartfiles -ffreestanding -I ../include

BUILD_DIR = ../build

C_FILES    = $(wildcard *.c)
OBJ_FILES  = $(C_FILES:%.c=$(BUILD_DIR)/%.o)

all: $(OBJ_FILES)

$(BUILD_DIR)/%.o: %.c
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ_FILES)