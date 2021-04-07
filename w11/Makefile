CC=gcc
CFLAGS=-Wall -Werror -Og

.PHONY: all
all: clock-example

clock-example: clock-example.c
	${CC} ${CFLAGS} $< -o $@

.PHONY: clean
clean:
	rm -f clock-example
