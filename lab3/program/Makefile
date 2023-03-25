CC      = aarch64-linux-gnu-gcc
LD      = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy

ASMFLAGS = -ffreestanding

BUILD_DIR = build

ASM_FILES = $(wildcard *.s)
OBJ_FILES = $(ASM_FILES:%.s=$(BUILD_DIR)/%.o)

ELF   = program.elf
PROGS = program.img

all: $(PROGS)

$(BUILD_DIR)/%.o: %.s
	$(shell [ ! -d $(@D) ] && mkdir -p $(@D))
	$(CC) $(ASMFLAGS) -c $< -o $@

$(BUILD_DIR)/$(ELF): $(OBJ_FILES)
	$(LD) -static -o $@ $(OBJ_FILES)

$(PROGS): $(BUILD_DIR)/$(ELF)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(PROGS) $(BUILD_DIR)