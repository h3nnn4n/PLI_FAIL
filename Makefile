CC=gcc
CFLAGS=-Wall -O2 -lglpk -lm -g
SOURCES=main.c matrix.c utils.c
BIN=main

.PHONY: clean

all:
	$(CC) $(SOURCES) -o $(BIN) $(CFLAGS) 

clean:
	-@rm -rf core $(BIN) 2> /dev/null || true
