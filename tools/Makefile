SRC := $(wildcard *.c)
ELF := $(SRC:.c=)

CFLAGS := -O3

all: clean $(ELF)

$(ELF): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(ELF)
