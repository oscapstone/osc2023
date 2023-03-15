SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
INCLUDES = -I ./include

all: $(OBJS)

%.o: %.c
	aarch64-linux-gnu-gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm *.o 2> /dev/null || true
