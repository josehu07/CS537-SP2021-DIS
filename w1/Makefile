CC=gcc
CFLAGS=-Wall -Werror -g


.PHONY: all
all: simple-cat


simple-cat: simple-cat.c
	$(CC) $< -o $@ $(CFLAGS)


.PHONY: clean
clean:
	rm -f simple-cat *.o
