SRCS 			= $(wildcard ./code/*.c)
OBJS			= $(SRCS:%.c=%.o)

PROGRAM			= fork_test
ELF				= ./code/$(PROGRAM).elf
IMG				= ../$(PROGRAM).img

CFLAGS 			= -Wall -ffreestanding -nostdlib
CC 				= aarch64-linux-gnu-gcc
LINKER 			= aarch64-linux-gnu-ld
OBJ_COPY 		= aarch64-linux-gnu-objcopy

.PHONY: 		clean

all:			$(IMG)

$(IMG):			$(OBJS) code/syscall.o
	$(LINKER) -nostdlib $(OBJS) code/syscall.o -T link.ld -o $(ELF)
	$(OBJ_COPY) -O binary $(ELF) $@

code/syscall.o:	code/syscall.S
	$(CC) $(CFLAGS) -c $< -o $@

code/%.o: 		code/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJS) code/syscall.o $(ELF) $(IMG) >/dev/null 2>/dev/null || true
	