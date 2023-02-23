CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

ELF = kernel8.elf
PROGS = kernel8.img

MACHINE = raspi3

all: $(PROGS)

$(PROGS): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): start.o linker.ld
	$(LD) -T linker.ld -o $@ start.o

%.o: %.s
	$(CC) -c $<

run: $(PROGS)
	$(QEMU) -M $(MACHINE) -kernel $< -display none -d in_asm

debug: $(PROGS)
	$(QEMU) -M $(MACHINE) -kernel $< -display none -S -s

clean:
	rm *.o *.elf $(PROGS)