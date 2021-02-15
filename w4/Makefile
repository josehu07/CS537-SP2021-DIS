
CC=gcc
CFLAGS=-Wall -Werror -O2


SOURCES=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,${SOURCES})


${TARGETS}: %:%.c
	${CC} ${CFLAGS} $< -o $@


.PHONY: clean
clean:
	rm -f ${TARGETS}
