
CC=gcc
CFLAGS=-Wall -Werror -O2


SOURCES=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,${SOURCES})


.PHONY: all
all: ${TARGETS}


${TARGETS}: %:%.c
	${CC} ${CFLAGS} $< -o $@


.PHONY: clean
clean:
	rm -f ${TARGETS}
